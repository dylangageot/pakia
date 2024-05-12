#pragma once
#include <stdint.h>

namespace ps2 {

    void begin();

    enum event_kind { PRESSED = 0, RELEASED = 1 };

    struct event {
        event_kind event_kind;
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

    enum command {
        RESET = 0xFF,
        RESEND = 0xFE,
        SET_DEFAULT = 0xF6,
        DISABLE = 0xF5,
        ENABLE = 0xF4,
        SET_TYPEMATIC_RATE_DELAY = 0xF3,
        ECHO = 0xEE,
        SET_RESET_LEDS = 0xED
    };

    void send(command command, uint8_t args = 0);

} // namespace ps2
