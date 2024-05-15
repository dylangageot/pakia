#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>

#include "amiga.hh"
#include "ps2.hh"
#include "scancodes.hh"
#include "translation_map.hh"

enum {
    COMBO_PRESSED = 7,
    RESET_REQUEST_SENT = 1 << 3,
    COMBO_RELEASED = 1 << 4
};

void setup() {
    MCUCR &= ~(1 << PUD);  // Enable pull-up resistors.
    MCUSR &= ~(1 << WDRF); // Reset Watchdog reset flag.
    wdt_disable();
    sei();
    ps2::begin();
    amiga::begin();
    ps2::send(ps2::command::RESET);
}

int main() {
    namespace scps2 = scancodes::ps2;
    namespace scami = scancodes::amiga;
    ps2::receiver receiver;
    ps2::event event;
    uint8_t last_scancode = 0;
    uint8_t reset_combo = 0;
    bool caps_lock = false;
    bool alt_num_pad = false;
    setup();
    while (1) {
        if (receiver.consume(event)) {
            uint8_t reset_key =
                (event.scancode == scps2::LEFT_CTRL ? 1 : 0) |
                (event.scancode == scps2::extended::LEFT_OS ? 2 : 0) |
                (event.scancode == scps2::extended::RIGHT_OS ? 4 : 0);

            // Detect reset combo (Ctrl + Left Amiga + Right Amiga).
            if (reset_key && !(reset_combo & COMBO_RELEASED)) {
                if (event.kind == ps2::event::PRESSED &&
                    !(reset_combo & RESET_REQUEST_SENT)) {
                    reset_combo |= reset_key;
                    if ((reset_combo & COMBO_PRESSED) == COMBO_PRESSED) {
                        amiga::hold_reset();
                        reset_combo |= RESET_REQUEST_SENT;
                    }
                } else if (event.kind == ps2::event::RELEASED) {
                    reset_combo &= ~reset_key;
                    if (reset_combo & RESET_REQUEST_SENT) {
                        amiga::release_reset();
                        reset_combo |= COMBO_RELEASED;
                    }
                }
            }

            if (amiga::is_ready()) {
                uint8_t amiga_scancode =
                    pgm_read_byte_near(ps2_to_amiga_scancode + event.scancode);

                if ((event.kind == ps2::event::PRESSED) &&
                    (event.scancode == last_scancode)) {
                    continue;
                }

                // Detect alternative key (Menu).
                alt_num_pad = event.scancode == scps2::extended::MENU
                                  ? event.kind == ps2::event::PRESSED
                                  : alt_num_pad;

                // Drop any unknown key.
                // Past this point, any unknown scancode on amiga side cannot be
                // used.
                if (amiga_scancode == 0xFF) {
                    continue;
                }

                // If + key is actually a - due to the alternative
                // keypad function.
                amiga_scancode =
                    alt_num_pad && (amiga_scancode == scami::NUM_PLUS)
                        ? scami::NUM_MINUS
                        : amiga_scancode;

                // Send key with its type (key up or key down).
                if (event.kind == ps2::event::PRESSED) {
                    last_scancode = event.scancode;
                    if (event.scancode != scps2::CAPS_LOCK) {
                        amiga::send(amiga_scancode);
                    } else {
                        caps_lock = !caps_lock;
                        amiga::send(
                            amiga_scancode |
                            (caps_lock ? amiga::KEY_DOWN : amiga::KEY_UP));
                        ps2::send(ps2::command::SET_RESET_LEDS,
                                  caps_lock ? ps2::leds::CAPS_LOCK : 0);
                    }
                } else if (event.kind == ps2::event::RELEASED) {
                    last_scancode =
                        event.scancode == last_scancode ? 0 : last_scancode;
                    if (event.scancode != scps2::CAPS_LOCK) {
                        amiga::send(amiga_scancode | amiga::KEY_UP);
                    }
                }
            }
        }
    }
}
