#pragma once

namespace amiga {

    void begin();
    bool send(uint8_t keycode);
    bool is_ready();

    enum fail_state { OK = 0, SEND_ONE_BIT, SEND_LOST_SYNC };

    enum sync_state { UNSYNCED = 0, INITIATE_STREAM, TERMINATE_STREAM };

    struct fsm {
        void (*state)();
        uint8_t data;
        uint8_t data_to_recover;
        uint8_t bit_counter;
        enum fail_state fail_state;
        enum sync_state sync_state;

        void begin();
        bool is_ready();
        bool trigger_send();
    };

} // namespace amiga
