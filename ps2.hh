#pragma once

const size_t PS2_FRAME_COUNT = 16;

const size_t PS2_CLK_PIN = 3;
const size_t PS2_DAT_PIN = 4;

struct ps2_frame_t
{
    bool available;
    uint8_t scancode;
};

extern volatile ps2_frame_t ps2_frames[PS2_FRAME_COUNT];

struct ps2_frame_iterator_t
{
    ps2_frame_iterator_t(volatile ps2_frame_t *frame = ps2_frames, const size_t frame_count = PS2_FRAME_COUNT) : _frame(frame), _initial_frame(frame), _frame_count(frame_count) {}

    inline volatile ps2_frame_t *get()
    {
        return _frame;
    }

    inline void move_forward()
    {
        _frame = (++_frame >= (_initial_frame + _frame_count)) ? _initial_frame : _frame;
    }

private:
    volatile ps2_frame_t *_frame;
    volatile ps2_frame_t *_initial_frame;
    const size_t _frame_count;
};

struct ps2_fsm_t
{
    void (*state)();
    uint8_t buffer;
    uint8_t counter;
    ps2_frame_iterator_t iterator;

    void begin();
};

extern ps2_fsm_t ps2_fsm;

enum event_kind_t
{
    PRESSED = 0,
    RELEASED = 1
};

struct ps2_event_t
{
    enum event_kind_t event_kind;
    uint8_t scancode;
};

struct ps2_parser_t
{
    ps2_parser_t();
    bool consume(ps2_frame_iterator_t *iterator, ps2_event_t *event);
    void reset();

private:
    bool _is_released;
    bool _is_e0_prefixed;
    uint8_t _last_scancode_fed;
};
