#pragma once

namespace amiga {

    void begin();
    bool is_ready();
    bool send(uint8_t keycode);

    enum fail_state { OK = 0, SEND_ONE_BIT, SEND_LOST_SYNC };

    enum sync_state { UNSYNCED = 0, INITIATE_STREAM, TERMINATE_STREAM };

} // namespace amiga
