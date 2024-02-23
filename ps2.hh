#pragma once

namespace ps2
{

    const size_t FRAME_COUNT = 16;

    const size_t CLK_PIN = 3;
    const size_t DAT_PIN = 4;

    struct frame
    {
        bool available;
        uint8_t scancode;
    };

    extern volatile frame frames[FRAME_COUNT];

    struct frame_iterator
    {
        frame_iterator(volatile frame *frame = frames, const size_t frame_count = FRAME_COUNT) : _frame(frame), _initial_frame(frame), _frame_count(frame_count) {}

        inline volatile frame *get()
        {
            return _frame;
        }

        inline void move_forward()
        {
            _frame = (++_frame >= (_initial_frame + _frame_count)) ? _initial_frame : _frame;
        }

    private:
        volatile frame *_frame;
        volatile frame *_initial_frame;
        const size_t _frame_count;
    };

    struct fsm
    {
        void (*state)();
        uint8_t buffer;
        uint8_t counter;
        frame_iterator iterator;

        void begin();
    };

    extern struct fsm fsm;

    enum event_kind
    {
        PRESSED = 0,
        RELEASED = 1
    };

    struct event
    {
        enum event_kind event_kind;
        uint8_t scancode;
    };

    struct parser
    {
        parser();
        bool consume(frame_iterator *iterator, event *event);
        void reset();

    private:
        bool _is_released;
        bool _is_e0_prefixed;
        uint8_t _last_scancode_fed;
    };

}