#pragma once

namespace amiga {

    void begin();
    bool is_ready();
    bool send(uint8_t keycode);
    void hold_reset();
    void release_reset();

    const uint8_t KEY_DOWN = 0;
    const uint8_t KEY_UP = 1 << 7;

} // namespace amiga
