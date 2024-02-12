#pragma once

const size_t PS2_CLK_PIN = 3;
const size_t PS2_DAT_PIN = 4;

struct ps2_frame
{
    bool available;
    uint8_t key;
};

struct ps2_fsm
{
    void (*state)();
    uint8_t buffer;
    uint8_t counter;

    void begin();
};

extern volatile struct ps2_frame ps2_frame;
extern volatile struct ps2_fsm ps2_fsm;
