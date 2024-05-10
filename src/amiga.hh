#pragma once

namespace amiga {

    void begin();
    bool is_ready();
    bool send(uint8_t keycode);
    void hold_reset();
    void release_reset();

} // namespace amiga
