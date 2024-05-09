#pragma once

namespace amiga {

    void begin();
    bool is_ready();
    bool send(uint8_t keycode);
    void hold_reset();
    void release_reset();

    enum fail_state { OK = 0, SEND_ONE_BIT, SEND_LOST_SYNC };

    enum sync_state { UNSYNCED = 0, INITIATE_STREAM, TERMINATE_STREAM, FIRST_RESET_WARNING, SECOND_RESET_WARNING, RESET };

} // namespace amiga
