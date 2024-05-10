#include <avr/interrupt.h>
#include <avr/io.h>

#include "circular_buffer.hh"
#include "pins.hh"
#include "ps2.hh"

namespace ps2 {

    struct circular_buffer<uint8_t, 8> scancodes;

    void (*state)();
    uint8_t data;
    uint8_t counter;

    static inline void set_pins_pull_up() {
        DDRB &= ~(pins::ps2::CLK | pins::ps2::DAT);
        PORTB |= pins::ps2::CLK | pins::ps2::DAT;
    }

    static inline void set_clk_low() {
        PORTB &= ~pins::ps2::CLK;
        DDRB |= pins::ps2::CLK;
    }

    static inline void setup_clk_detection_int() {
        MCUCR = (MCUCR & ~((1 << ISC01) | (1 << ISC00))) | (1 << ISC01);
    }

    static inline void enable_clk_detection_int() {
        GIFR |= (1 << INTF0);
        GIMSK |= (1 << INT0);
    }

    static inline void disable_clk_detection_int() { GIMSK &= ~(1 << INT0); }

    static inline void setup_timer() {
        TCCR0A = (1 << WGM01);
        TCCR0B = (1 << CS02) | (1 << CS00);
        TCNT0 = 155;
    }

    static inline void enable_timer_int() {
        TIFR |= (1 << OCF0A);
        TIMSK |= (1 << OCIE0A);
    }

    static inline void disable_timer_int() { TIMSK &= ~(1 << OCIE0A); }

    static inline void inhibit() {
        // lets restore the communications in 20ms
        disable_clk_detection_int();
        set_clk_low();
        setup_timer();
        enable_timer_int();
    }

    static void process_start_bit();
    static void process_stop_bit() {
        if (!(PINB & pins::ps2::DAT)) {
            inhibit();
            return;
        }
        if (scancodes.write(data)) {
            state = process_start_bit;
        } else {
            // if circular data is full, inhibit keyboard scancode reception
            inhibit();
        }
    }

    static void process_parity_bit() {
        uint8_t read_parity = (PINB & pins::ps2::DAT) >> pins::ps2::DAT_PIN;
        uint8_t computed_parity = data;
        computed_parity ^= computed_parity >> 4;
        computed_parity ^= computed_parity >> 2;
        computed_parity ^= computed_parity >> 1;
        computed_parity = (~computed_parity) & 1;
        if (read_parity != computed_parity) {
            inhibit();
            return;
        }
        counter++;
        state = process_stop_bit;
    }

    static void process_data_bits() {
        data |= ((PINB & pins::ps2::DAT) >> pins::ps2::DAT_PIN) << counter++;
        state = (counter < 8) ? process_data_bits : process_parity_bit;
    }

    static void process_start_bit() {
        if (PINB & pins::ps2::DAT) {
            inhibit();
            return;
        }
        data = 0;
        counter = 0;
        state = process_data_bits;
    }


    void begin() {
        state = process_start_bit;
        set_pins_pull_up();
        setup_clk_detection_int();
        enable_clk_detection_int();
    }

    ISR(INT0_vect) { (*state)(); }

    ISR(TIM0_COMPA_vect) {
        disable_timer_int();
        state = process_start_bit;
        set_pins_pull_up();
        enable_clk_detection_int();
    }

    receiver::receiver() { reset(); }

    void receiver::reset() {
        _is_e0_prefixed = false;
        _is_released = false;
        _last_scancode_fed = 0;
    }

    bool receiver::consume(event &event) {
        if (scancodes.read(_last_scancode_fed)) {
            if (_last_scancode_fed == 0xF0) {
                _is_released = true;
            } else if (_last_scancode_fed == 0xE0) {
                _is_e0_prefixed = true;
            } else {
                event.event_kind = _is_released ? RELEASED : PRESSED;
                event.scancode =
                    _last_scancode_fed | (_is_e0_prefixed ? 1 << 7 : 0);
                reset();
                return event.scancode != 0;
            }
        }
        return false;
    }

} // namespace ps2
