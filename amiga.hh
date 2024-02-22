#pragma once

const size_t AMIGA_CLK_PIN = 5;
const size_t AMIGA_DAT_PIN = 2;

enum amiga_fail_state
{
    OK = 0,
    SEND_ONE_BIT,
    SEND_LOST_SYNC
};

enum amiga_sync_state
{
    UNSYNCED = 0,
    INITIATE_STREAM,
    TERMINATE_STREAM
};

struct amiga_fsm
{
    void (*state)();
    uint8_t data;
    uint8_t data_to_recover;
    uint8_t bit_counter;
    enum amiga_fail_state fail_state;
    enum amiga_sync_state sync_state;

    void begin();
    bool send_keycode(uint8_t keycode);
};

extern struct amiga_fsm amiga_fsm;
