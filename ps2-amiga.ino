
#include "ps2.hh"

void setup()
{
    Serial.begin(115200);
    ps2_fsm.begin();
}

void loop()
{
    uint8_t frame_index = 0;
    char s[80];
    volatile struct ps2_frame *frame = (ps2_frames + (ps2_fsm.frame_buffer_index++ & (PS2_FRAME_COUNT_POW_2 - 1)));
    if (frame->available)
    {
        uint8_t key = frame->key;
        frame->available = false;
        frame_index++;

        sprintf(s, "Received key: 0x%x\n", key);
        Serial.println(s);
    }
}