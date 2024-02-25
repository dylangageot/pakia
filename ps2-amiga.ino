#define __AVR_ATmega328P__
#include <avr/io.h>

#include "ps2.hh"
#include "amiga.hh"
#include "translation_map.hh"

void setup()
{
    Serial.begin(115200);
    ps2::begin();
    amiga::begin();
}

void loop()
{
    static ps2::receiver receiver;
    static uint8_t last_scancode = 0;
    static bool caps_lock = false;
    static char s[80];

    ps2::event event;
    while (amiga::is_ready() && receiver.consume(event))
    {
        sprintf(s, "Key %s: 0x%x\n", ((event.event_kind == ps2::event_kind::PRESSED) ? "pressed" : "released"), event.scancode);
        Serial.println(s);

        uint8_t amiga_keycode = pgm_read_byte_near(ps2_scancode_to_amiga_keycode + event.scancode);
        if ((event.event_kind == ps2::event_kind::PRESSED) && (event.scancode != last_scancode))
        {
            last_scancode = event.scancode;
            if (event.scancode == ps2::CAPS_LOCK_SCANCODE)
            {
                caps_lock = !caps_lock;
                amiga::send(amiga_keycode | (caps_lock ? (1 << 7) : 0)) &&
                    Serial.println(F("caps lock inversed!"));
            }
            else
            {
                amiga::send(amiga_keycode) &&
                    Serial.println(F("pressed key sent to amiga!"));
            }
        }
        else if (event.event_kind == ps2::event_kind::RELEASED)
        {
            if (event.scancode == last_scancode) {
                last_scancode = 0;
            }
            if (event.scancode != ps2::CAPS_LOCK_SCANCODE)
            {
                amiga::send(amiga_keycode | 1 << 7) &&
                    Serial.println(F("released key sent to amiga!"));
            }
        }
    }
}