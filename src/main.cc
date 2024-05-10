#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>

#include "amiga.hh"
#include "ps2.hh"
#include "scancodes.hh"
#include "translation_map.hh"

int main() {
    // Enable pull-up resistors.
    MCUCR &= ~(1 << PUD);
    // Set to zero previous watch-dog trigerring flag and disable watch-dog
    // timer.
    MCUSR &= ~(1 << WDRF);
    wdt_disable();
    // Enable interrupts.
    sei();

    ps2::begin();
    amiga::begin();

    ps2::receiver receiver;
    ps2::event event;

    uint8_t last_scancode = 0;
    uint8_t reset_combo = 0;
    bool caps_lock = false;
    bool alt_num_pad = false;

    while (1) {
        if (receiver.consume(event)) {

            { // Reset detection
                namespace scps2 = scancodes::ps2;
                uint8_t reset_key =
                    (event.scancode == scps2::LEFT_CTRL ? 1 : 0) |
                    (event.scancode == scps2::extended::LEFT_OS ? 2 : 0) |
                    (event.scancode == scps2::extended::RIGHT_OS ? 4 : 0);

                enum {
                    COMBO_PRESSED = 7,
                    RESET_REQUEST_SENT = 1 << 3,
                    COMBO_RELEASED = 1 << 4
                };

                // Detect reset combo (Ctrl + Left Amiga + Right Amiga).
                if (reset_key && !(reset_combo & COMBO_RELEASED)) {
                    if (event.event_kind == ps2::event_kind::PRESSED &&
                        !(reset_combo & RESET_REQUEST_SENT)) {
                        reset_combo |= reset_key;
                        if ((reset_combo & COMBO_PRESSED) == COMBO_PRESSED) {
                            amiga::hold_reset();
                            reset_combo |= RESET_REQUEST_SENT;
                        }
                    } else if (event.event_kind == ps2::event_kind::RELEASED) {
                        reset_combo &= ~reset_key;
                        if (reset_combo & RESET_REQUEST_SENT) {
                            amiga::release_reset();
                            reset_combo |= COMBO_RELEASED;
                        }
                    }
                }
            }

            if (amiga::is_ready()) {
                // Translate PS/2 to Amiga scancode.
                uint8_t amiga_scancode =
                    pgm_read_byte_near(ps2_to_amiga_scancode + event.scancode);

                if (event.scancode == scancodes::ps2::extended::MENU) {
                    alt_num_pad = event.event_kind == ps2::event_kind::PRESSED;
                }

                // Drop any unknown key.
                // Backquote correspondance in Amiga scancode is 0x00, we need
                // to ensure that backquote can be pressed and not misread for
                // an unknown key.
                if ((event.scancode != scancodes::ps2::BACKQUOTE) &&
                    (amiga_scancode == 0)) {
                    continue;
                }

                // If + key is actually a - due to the alternative keypad
                // function.
                if ((amiga_scancode == scancodes::amiga::NUM_PLUS) &&
                    alt_num_pad) {
                    amiga_scancode = scancodes::amiga::NUM_MINUS;
                }

                if ((event.event_kind == ps2::event_kind::PRESSED) &&
                    (event.scancode != last_scancode)) {
                    last_scancode = event.scancode;
                    if (event.scancode == scancodes::ps2::CAPS_LOCK) {
                        caps_lock = !caps_lock;
                        amiga::send(amiga_scancode | (caps_lock ? 1 << 7 : 0));
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
}
