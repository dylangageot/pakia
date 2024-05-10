#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>

#include "amiga.hh"
#include "circular_buffer.hh"
#include "pins.hh"

namespace amiga {

    const uint8_t lost_sync_code = 0xF9;
    const uint8_t initiate_power_up_stream_code = 0xFD;
    const uint8_t terminate_power_up_stream_code = 0xFE;
    const uint8_t reset_warning = 0x78;

    enum sync_state {
        UNSYNCED = 0,
        INITIATE_STREAM,
        TERMINATE_STREAM,
        SYNCED,
        SEND_ONE_BIT,
        SEND_LOST_SYNC,
        SEND_LOST_DATA,
        FIRST_RESET_WARNING,
        SECOND_RESET_WARNING,
        TIMER_READY_HARD_RESET
    };

    static struct circular_buffer<uint8_t, 8> scancodes;

    static void (*state)();
    static uint8_t data;
    static uint8_t data_to_recover;
    static uint8_t counter;
    static enum sync_state sync_state;
    static uint8_t user_ready_hard_reset;

    static inline void setup_reset_pin() {
        DDRB &= ~pins::amiga::RST;
        PORTB &= ~pins::amiga::RST;
    }

    static inline void setup_timer() {
        TCCR1 = (1 << PWM1A);
        GTCCR &= (1 << TSM) | (1 << PSR0);
    }

    static inline void setup_timer_for_frame() {
        TCCR1 =
            (TCCR1 & ~((1 << CS13) | (1 << CS12) | (1 << CS11) | (1 << CS10))) |
            (1 << CS10);
        OCR1C = 159;
        TCNT1 = 0;
    }

    static inline void setup_timer_for_resync() {
        TCCR1 =
            (TCCR1 & ~((1 << CS13) | (1 << CS12) | (1 << CS11) | (1 << CS10))) |
            (1 << CS13) | (1 << CS12) | (1 << CS11); // 8192 prescaler
        OCR1C = 138;
        TCNT1 = 0;
    }

    static inline void setup_timer_for_hard_reset() {
        TCCR1 =
            (TCCR1 & ~((1 << CS13) | (1 << CS12) | (1 << CS11) | (1 << CS10))) |
            (1 << CS13) | (1 << CS12) | (1 << CS11); // 8192 prescaler
        OCR1C = 243;
        TCNT1 = 0;
    }

    static inline void enable_timer_int() {
        TIFR |= (1 << TOV1);
        TIMSK |= (1 << TOIE1);
    }

    static inline void disable_timer_int() { TIMSK &= ~(1 << TOIE1); }

    static inline void set_pins_as_pull_up() {
        DDRB &= ~(pins::amiga::CLK | pins::amiga::DAT);
        PORTB |= pins::amiga::CLK | pins::amiga::DAT;
    }

    static inline void set_pins_as_output() {
        DDRB |= pins::amiga::CLK | pins::amiga::DAT;
        PORTB |= pins::amiga::CLK | pins::amiga::DAT;
    }

    static inline void set_dat_bit(uint8_t bit) {
        PORTB = (PORTB & ~pins::amiga::DAT) | ((bit) ? pins::amiga::DAT : 0);
    }

    static inline void set_clk_high() { PORTB |= pins::amiga::CLK; }

    static inline void set_clk_low() { PORTB &= ~pins::amiga::CLK; }

    static inline void enable_ack_detection_int() {
        GIFR |= (1 << PCIF);
        GIMSK |= (1 << PCIE);
    }

    static inline void disable_ack_detection_int() { GIMSK &= ~(1 << PCIE); }

    static inline void setup_ack_detection() {
        PCMSK |= (1 << PCINT4);
        disable_ack_detection_int();
    }

    static inline void try_hard_reset() {
        if ((sync_state == TIMER_READY_HARD_RESET) && user_ready_hard_reset) {
            set_clk_high();
            cli();
            wdt_enable(WDTO_15MS);
            wdt_reset();
            while (true) {
            }
        }
    }

    static inline uint8_t roll_and_inverse_data(const uint8_t &data) {
        return ~((data << 1) | (data >> 7));
    }

    static void frame_set_dat_bit();

    static inline void send_scancode(const uint8_t &to_send,
                                     const uint8_t &roll_and_inverse = 1,
                                     const uint8_t &bit_count = 8) {
        data = roll_and_inverse ? roll_and_inverse_data(to_send) : to_send;
        counter = 8 - bit_count;
        state = frame_set_dat_bit;
        setup_timer_for_frame();
        set_pins_as_output();
        enable_timer_int();
    }

    static void frame_set_clk_low();

    static void frame_set_dat_bit() {
        set_dat_bit(data & (128 >> counter));
        state = frame_set_clk_low;
    }

    static void frame_set_clk_high();

    static void frame_set_clk_low() {
        set_clk_low();
        state = frame_set_clk_high;
    }

    static void end_transfer();

    static void frame_set_clk_high() {
        set_clk_high();
        state = (++counter < 8) ? frame_set_dat_bit : end_transfer;
    }

    static void wait_for_500_ms() {
        counter++;
        if (counter > 2) {
            disable_timer_int();
            sync_state = TIMER_READY_HARD_RESET;
            try_hard_reset();
        }
    }

    static void hard_reset() {
        set_pins_as_output();
        set_clk_low();
        counter = 0;
        state = wait_for_500_ms;
        enable_timer_int();
    }

    static void send_one_bit() {
        disable_timer_int();
        disable_ack_detection_int();
        if (sync_state != UNSYNCED) {
            sync_state = SEND_ONE_BIT;
            data_to_recover = data;
        }
        send_scancode(0, 0, 1);
    }

    static void end_transfer() {
        disable_timer_int();
        set_pins_as_pull_up();
        state = send_one_bit;
        setup_timer_for_resync();
        if (sync_state == FIRST_RESET_WARNING) {
            state = hard_reset;
        } else if (sync_state == SECOND_RESET_WARNING) {
            state = hard_reset;
            setup_timer_for_hard_reset();
        }
        enable_timer_int();
        enable_ack_detection_int();
    }

    static void wait_for_hard_reset() {
        if (++counter > 40) { // 40 * 250ms = 10s.
            disable_timer_int();
            disable_ack_detection_int();
            hard_reset();
        }
    }

    static void idle() {}

    ISR(TIM1_OVF_vect) { (*state)(); }

    ISR(PCINT0_vect) {
        // on rising edge of the acknowledment period from the Amiga
        if (PINB & pins::amiga::DAT) {
            disable_ack_detection_int();
            disable_timer_int();
            switch (sync_state) {
            case UNSYNCED:
                sync_state = INITIATE_STREAM;
                send_scancode(initiate_power_up_stream_code);
                break;
            case INITIATE_STREAM:
                sync_state = TERMINATE_STREAM;
                send_scancode(terminate_power_up_stream_code);
                break;
            case TERMINATE_STREAM:
            case SEND_LOST_DATA:
            case SYNCED:
                sync_state = SYNCED;
                if (scancodes.read(data)) {
                    send_scancode(data);
                } else {
                    state = idle;
                }
                break;
            case SEND_ONE_BIT:
                sync_state = SEND_LOST_SYNC;
                send_scancode(lost_sync_code);
                break;
            case SEND_LOST_SYNC:
                sync_state = SEND_LOST_DATA;
                send_scancode(data_to_recover,
                              0 /* do not roll and inverse bit */);
                break;
            case FIRST_RESET_WARNING:
                sync_state = SECOND_RESET_WARNING;
                send_scancode(reset_warning);
                break;
            case SECOND_RESET_WARNING:
                hard_reset();
            default:
                break;
            }
        } else if (sync_state == SECOND_RESET_WARNING) {
            // on falling edge of the second reset warning acknowledgement
            disable_ack_detection_int();
            disable_timer_int();
            state = wait_for_hard_reset;
            counter = 0;
            setup_timer_for_hard_reset();
            enable_timer_int();
            enable_ack_detection_int();
        }
    }

    void begin() {
        setup_ack_detection();
        setup_timer();
        setup_reset_pin();
        setup_timer_for_resync();
        set_pins_as_pull_up();
        sync_state = UNSYNCED;
        user_ready_hard_reset = 0;
        send_one_bit();
    }

    bool is_ready() { return sync_state == SYNCED; }

    bool send(uint8_t keycode) {
        if (scancodes.write(keycode)) {
            if (state == idle) {
                scancodes.read(keycode);
                send_scancode(keycode);
            }
            return true;
        } else {
            return false;
        }
    }

    void drain() {
        while (is_ready() && state != idle) {}
    }

    void hold_reset() {
        drain();
        sync_state = FIRST_RESET_WARNING;
        send_scancode(reset_warning);
    }

    void release_reset() {
        user_ready_hard_reset = 1;
        try_hard_reset();
    }

} // namespace amiga
