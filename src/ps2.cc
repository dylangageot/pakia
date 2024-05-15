#include <avr/interrupt.h>
#include <avr/io.h>

#include "circular_buffer.hh"
#include "pins.hh"
#include "ps2.hh"

namespace ps2 {

    enum direction { RECEIVING = 0, SENDING };

    struct circular_buffer<uint8_t, 8> scancodes;

    void (*state)();
    direction direction;
    uint8_t data;
    uint8_t counter;
    uint8_t cmd[2];
    bool has_arg;
    uint8_t byte_counter;

    static inline void set_pins_pull_up() {
        DDRB &= ~(pins::ps2::CLK | pins::ps2::DAT);
        PORTB |= pins::ps2::CLK | pins::ps2::DAT;
    }

    static inline void set_clk_pull_up() {
        DDRB &= ~(pins::ps2::CLK);
        PORTB |= pins::ps2::CLK;
    }

    static inline void set_clk_low() {
        PORTB &= ~pins::ps2::CLK;
        DDRB |= pins::ps2::CLK;
    }

    static inline void set_dat_pull_up() {
        DDRB &= ~(pins::ps2::DAT);
        PORTB |= pins::ps2::DAT;
    }

    static inline void set_dat_low() {
        PORTB &= ~pins::ps2::DAT;
        DDRB |= pins::ps2::DAT;
    }

    static inline void setup_clk_detection_int_as_falling() {
        MCUCR = (MCUCR & ~((1 << ISC01) | (1 << ISC00))) | (1 << ISC01);
    }

    static inline void setup_clk_dectection_int_as_rising() {
        MCUCR = (MCUCR & ~((1 << ISC01) | (1 << ISC00))) | (1 << ISC01) |
                (1 << ISC00);
    };

    static inline void enable_clk_detection_int() {
        GIFR |= (1 << INTF0);
        GIMSK |= (1 << INT0);
    }

    static inline void disable_clk_detection_int() { GIMSK &= ~(1 << INT0); }

    static inline void setup_timer_for_inhibition() {
        TCCR0A = (1 << WGM01);
        TCCR0B = (1 << CS02) | (1 << CS00);
        TCNT0 = 155;
    }

    static inline void setup_timer_for_rts() {
        TCCR0A = (1 << WGM01);
        TCCR0B = (1 << CS01);
        TCNT0 = 101;
    }

    static inline void enable_timer_int() {
        TIFR |= (1 << OCF0A);
        TIMSK |= (1 << OCIE0A);
    }

    static inline void disable_timer_int() { TIMSK &= ~(1 << OCIE0A); }

    static inline void inhibit(enum direction dir = RECEIVING) {
        direction = dir;
        // lets restore the communications in 20ms, or 100us if request-to-send
        disable_clk_detection_int();
        set_clk_low();
        set_dat_pull_up();
        if (dir == RECEIVING) {
            setup_timer_for_inhibition();
        } else if (dir == SENDING) {
            data = cmd[byte_counter++];
            setup_timer_for_rts();
        }
        enable_timer_int();
    }

    static void wait_for_idle() {
        setup_clk_detection_int_as_falling();
        inhibit(SENDING);
    }

    static void receive_start_bit();

    static void receive_stop_bit() {
        const uint8_t ACK = 0xFA;
        if (!(PINB & pins::ps2::DAT)) {
            inhibit();
            return;
        }
        if (direction == SENDING) {
            if (data == ACK) {
                if (has_arg && byte_counter < 2) {
                    setup_clk_dectection_int_as_rising();
                    state = wait_for_idle;
                } else {
                    direction = RECEIVING;
                    state = receive_start_bit;
                }
            } else if (data == RESEND) {
                byte_counter = 0;
                setup_clk_dectection_int_as_rising();
                state = wait_for_idle;
            } else {
                inhibit();
            }
        } else if (direction == RECEIVING) {
            if (scancodes.write(data)) {
                state = receive_start_bit;
            } else {
                // if circular data is full, inhibit keyboard scancode reception
                inhibit();
            }
        }
    }

    static void receive_parity_bit() {
        uint8_t read_parity = (PINB & pins::ps2::DAT) >> pins::ps2::DAT_POS;
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
        state = receive_stop_bit;
    }

    static void receive_data_bits() {
        data |= ((PINB & pins::ps2::DAT) >> pins::ps2::DAT_POS) << counter++;
        state = counter < 8 ? receive_data_bits : receive_parity_bit;
    }

    static void receive_start_bit() {
        if (PINB & pins::ps2::DAT) {
            inhibit();
            return;
        }
        data = 0;
        counter = 0;
        state = receive_data_bits;
    }

    static void receive_ack_bit() {
        if (PINB & pins::ps2::DAT) {
            inhibit();
        } else {
            state = receive_start_bit;
        }
    }

    static void send_stop_bit() {
        set_dat_pull_up();
        state = receive_ack_bit;
    }

    static void send_parity_bit() {
        uint8_t computed_parity = data;
        computed_parity ^= computed_parity >> 4;
        computed_parity ^= computed_parity >> 2;
        computed_parity ^= computed_parity >> 1;
        computed_parity = (~computed_parity) & 1;
        PORTB =
            (PORTB & ~pins::ps2::DAT) | (computed_parity ? pins::ps2::DAT : 0);
        state = send_stop_bit;
    }

    static void send_data_bits() {
        PORTB = (PORTB & ~pins::ps2::DAT) |
                ((data & (1 << counter++)) ? pins::ps2::DAT : 0);
        state = (counter < 8) ? send_data_bits : send_parity_bit;
    }

    ISR(INT0_vect) { (*state)(); }

    ISR(TIM0_COMPA_vect) {
        if (direction == SENDING) { // Request-To-Send?
            disable_timer_int();
            set_dat_low();
            counter = 0;
            state = send_data_bits;
            set_clk_pull_up();
            enable_clk_detection_int();
        } else if (direction == RECEIVING) {
            disable_timer_int();
            state = receive_start_bit;
            set_pins_pull_up();
            enable_clk_detection_int();
        }
    }

    void begin() {
        state = receive_start_bit;
        direction = RECEIVING;
        has_arg = false;
        set_pins_pull_up();
        setup_clk_detection_int_as_falling();
        enable_clk_detection_int();
    }

    void send(command command, uint8_t cmd_arg) {
        // we need to perform a request-to-send, which implies:
        // 1. inhibiting KCLK for a least 100us, i.e. 5*20ms
        // 2. putting KDAT to low while raising KCLK
        // 3. send the command bits on the falling edge with parity and stop bit
        // 4. check that there is an ack bit
        // 5. check for ack bytes
        // if there is an args, send args, then check for ack bytes too
        while (direction == SENDING) {
        }
        byte_counter = 0;
        cmd[0] = command;
        if ((command == SET_TYPEMATIC_RATE_DELAY) ||
            (command == SET_RESET_LEDS)) {
            cmd[1] = cmd_arg;
            has_arg = true;
        } else {
            has_arg = false;
        }
        inhibit(SENDING);
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
                event.kind = _is_released ? event::RELEASED : event::PRESSED;
                event.scancode =
                    _last_scancode_fed | (_is_e0_prefixed ? 1 << 7 : 0);
                reset();
                return event.scancode != 0;
            }
        }
        return false;
    }

} // namespace ps2
