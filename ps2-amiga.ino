#define __AVR_ATmega328P__
#include <avr/io.h>

#include "ps2.hh"
#include "amiga.hh"

static char translation_map[128];

static bool key_status[128] = {false};

void init_translation_map()
{
    // first line
    translation_map[0x15] = 0x10; // Q
    translation_map[0x1D] = 0x11; // W
    translation_map[0x24] = 0x12; // E
    translation_map[0x2D] = 0x13; // R
    translation_map[0x2C] = 0x14; // T
    translation_map[0x35] = 0x15; // Y
    translation_map[0x3C] = 0x16; // U
    translation_map[0x43] = 0x17; // I
    translation_map[0x44] = 0x18; // O
    translation_map[0x4D] = 0x19; // P

    // second line
    translation_map[0x1C] = 0x20; // A
    translation_map[0x1B] = 0x21; // S
    translation_map[0x23] = 0x22; // D
    translation_map[0x2B] = 0x23; // F
    translation_map[0x34] = 0x24; // G
    translation_map[0x33] = 0x25; // H
    translation_map[0x3B] = 0x26; // J
    translation_map[0x42] = 0x27; // K
    translation_map[0x4B] = 0x28; // L

    // third line
    translation_map[0x1A] = 0x31; // X
    translation_map[0x22] = 0x32; // A
    translation_map[0x21] = 0x33; // C
    translation_map[0x2A] = 0x34; // V
    translation_map[0x32] = 0x35; // B
    translation_map[0x31] = 0x36; // M
}

bool released = false;

void setup()
{
    Serial.begin(115200);
    ps2_fsm.begin();
    amiga_fsm.begin(); 
    init_translation_map();
    released = false;
}

void loop()
{
    uint8_t frame_index = 0;
    char s[80];
    volatile struct ps2_frame *frame = (ps2_frames + (ps2_fsm.frame_buffer_index++ & (PS2_FRAME_COUNT_POW_2 - 1)));
    if (frame->available)
    {
        uint8_t scancode = frame->key;
        frame->available = false;
        frame_index++;

        sprintf(s, "Received key: 0x%x\n", scancode);
        Serial.println(s);

        if (scancode == 0xF0)
        {
            released = true;
        }
        else
        {
            uint8_t keycode = translation_map[scancode];
            if (released)
            {
                amiga_fsm.send_keycode(keycode | (1 << 7));
                released = false;
                key_status[keycode] = false;
            }
            else if (key_status[keycode] == false)
            {
                amiga_fsm.send_keycode(keycode);
                key_status[keycode] == true;
            }
        }
    }
}