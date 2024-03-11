#include <avr/io.h>
#include <avr/interrupt.h>

#include "ps2.hh"
#include "amiga.hh"
#include "translation_map.hh"

int main()
{

    SREG |= (1 << SREG_I);
    MCUCR &= ~(1 << PUD);

    ps2::begin();
    amiga::begin();

    ps2::receiver receiver;
    uint8_t last_scancode = 0;
    bool caps_lock = false;
    char s[80];
    ps2::event event;
    while (1)
    {
        while (amiga::is_ready() && receiver.consume(event))
        {
            uint8_t amiga_keycode = pgm_read_byte_near(ps2_scancode_to_amiga_keycode + event.scancode);
            if ((event.event_kind == ps2::event_kind::PRESSED) && (event.scancode != last_scancode))
            {
                last_scancode = event.scancode;
                if (event.scancode == ps2::CAPS_LOCK_SCANCODE)
                {
                    caps_lock = !caps_lock;
                    amiga::send(amiga_keycode | (caps_lock ? (1 << 7) : 0));
                }
                else
                {
                    amiga::send(amiga_keycode);
                }
            }
            else if (event.event_kind == ps2::event_kind::RELEASED)
            {
                if (event.scancode == last_scancode)
                {
                    last_scancode = 0;
                }
                if (event.scancode != ps2::CAPS_LOCK_SCANCODE)
                {
                    amiga::send(amiga_keycode | 1 << 7);
                }
            }
        }
    }
}