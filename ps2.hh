#pragma once

namespace ps2
{
    
    const size_t CLK_PIN = 3;
    const size_t DAT_PIN = 4;

    void begin();

    struct frame
    {
        bool available;
        uint8_t scancode;
    };

    template <typename T, size_t SIZE = 16>
    struct circular_buffer
    {
        struct iterator
        {
            iterator(volatile T *begin_ptr) : _elem_ptr(begin_ptr), _begin_ptr(begin_ptr), _end_ptr(begin_ptr + SIZE) {}

            inline volatile T *get()
            {
                return _elem_ptr;
            }

            inline void next()
            {
                _elem_ptr = (++_elem_ptr = _end_ptr) ? _begin_ptr : _elem_ptr;
            }

        private:
            volatile T *_elem_ptr;
            volatile T *_begin_ptr;
            volatile T *_end_ptr;
        };

        inline iterator &write_iterator()
        {
            return _write_iterator;
        }

        inline iterator &read_iterator()
        {
            return _read_iterator;
        }

    private:
        volatile T _buffer[SIZE];
        iterator _write_iterator = iterator(_buffer);
        iterator _read_iterator = iterator(_buffer);
    };

    struct fsm
    {
        void (*state)();
        uint8_t buffer;
        uint8_t counter;

        void begin();
    };

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

    struct receiver
    {
        receiver();
        bool consume(event &event);
        void reset();

    private:
        bool _is_released;
        bool _is_e0_prefixed;
        uint8_t _last_scancode_fed;
    };

}