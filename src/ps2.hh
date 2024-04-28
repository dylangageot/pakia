#pragma once

#include "circular_buffer.hh"

namespace ps2 {

    void begin();

    struct fsm {
        void (*state)();
        uint8_t buffer;
        uint8_t counter;

        void begin();
    };

    enum event_kind { PRESSED = 0, RELEASED = 1 };

    struct event {
        enum event_kind event_kind;
        uint8_t scancode;
    };

    struct receiver {
        receiver();
        bool consume(event &event);
        void reset();

      private:
        bool _is_released;
        bool _is_e0_prefixed;
        uint8_t _last_scancode_fed;
    };

} // namespace ps2