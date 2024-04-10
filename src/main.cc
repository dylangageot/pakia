#include <avr/interrupt.h>
#include <avr/io.h>

#include "amiga.hh"
#include "ps2.hh"
#include "scancodes.hh"
#include "translation_map.hh"

int main() {

    SREG |= (1 << SREG_I);
    MCUCR &= ~(1 << PUD);

    ps2::begin();
    amiga::begin();

    ps2::receiver receiver;
    uint8_t last_scancode = 0;
    bool caps_lock = false;
    bool alt_num_pad = false;
    while (1) {
        ps2::event event;
        while (amiga::is_ready() && receiver.consume(event)) {
            uint8_t amiga_scancode =
                pgm_read_byte_near(ps2_to_amiga_scancode + event.scancode);

            if (event.scancode == scancodes::ps2::extended::MENU) {
                alt_num_pad = event.event_kind == ps2::event_kind::PRESSED;
            }

            // drop any unknown key
            // backquote correspondance in Amiga scancode is 0, we need to
            // ensure that backquote can be pressed anyway
            if ((event.scancode != scancodes::ps2::BACKQUOTE) &&
                (amiga_scancode == 0)) {
                continue;
            }

            // if + key is actually a - due to the alternative keypad function
            if ((amiga_scancode == scancodes::amiga::NUM_PLUS) && alt_num_pad) {
                amiga_scancode = scancodes::amiga::NUM_MINUS;
            }

            if ((event.event_kind == ps2::event_kind::PRESSED) &&
                (event.scancode != last_scancode)) {
                last_scancode = event.scancode;
                if (event.scancode == scancodes::ps2::CAPS_LOCK) {
                    caps_lock = !caps_lock;
                    amiga::send(amiga_scancode | (caps_lock ? (1 << 7) : 0));
                } else {
                    amiga::send(amiga_scancode);
                }
            } else if (event.event_kind == ps2::event_kind::RELEASED) {
                if (event.scancode == last_scancode) {
                    last_scancode = 0;
                }
                if (event.scancode != scancodes::ps2::CAPS_LOCK) {
                    amiga::send(amiga_scancode | 1 << 7);
                }
            }
        }
    }
}