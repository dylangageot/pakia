#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>

#include "amiga.hh"
#include "circular_buffer.hh"
#include "pins.hh"

namespace amiga {

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
    static uint8_t bit_counter;
    static enum sync_state sync_state;
    static uint8_t user_ready_hard_reset;

    inline void setup_timer() {
        TCCR1 = (1 << PWM1A);
        GTCCR &= (1 << TSM) | (1 << PSR0);
    }

    inline void setup_timer_for_frame() {
        TCCR1 =
            (TCCR1 & ~((1 << CS13) | (1 << CS12) | (1 << CS11) | (1 << CS10))) |
            (1 << CS10);
        OCR1C = 159;
        TCNT1 = 0;
    }

    inline void setup_timer_for_resync() {
        // 8192 prescaler
        TCCR1 =
            (TCCR1 & ~((1 << CS13) | (1 << CS12) | (1 << CS11) | (1 << CS10))) |
            (1 << CS13) | (1 << CS12) | (1 << CS11);
        OCR1C = 138;
        TCNT1 = 0;
    }

    inline void setup_timer_for_hard_reset() {
        // 8192 prescaler
        TCCR1 =
            (TCCR1 & ~((1 << CS13) | (1 << CS12) | (1 << CS11) | (1 << CS10))) |
            (1 << CS13) | (1 << CS12) | (1 << CS11);
        OCR1C = 243;
        TCNT1 = 0;
    }

    inline void enable_timer_int() {
        TIFR |= (1 << TOV1);
        TIMSK |= (1 << TOIE1);
    }

    inline void disable_timer_int() { TIMSK &= ~(1 << TOIE1); }

    inline void set_pins_as_pull_up() {
        DDRB &= ~(pins::amiga::CLK | pins::amiga::DAT);
        PORTB |= pins::amiga::CLK | pins::amiga::DAT;
    }

    inline void set_pins_as_output() {
        DDRB |= pins::amiga::CLK | pins::amiga::DAT;
        PORTB |= pins::amiga::CLK | pins::amiga::DAT;
    }

    inline void set_dat_bit(uint8_t bit) {
        PORTB = (PORTB & ~pins::amiga::DAT) | ((bit) ? pins::amiga::DAT : 0);
    }

    inline void set_clk_high() { PORTB |= pins::amiga::CLK; }

    inline void set_clk_low() { PORTB &= ~pins::amiga::CLK; }

    inline void enable_ack_detection_int() {
        GIFR |= (1 << PCIF);
        GIMSK |= (1 << PCIE);
    }

    inline void disable_ack_detection_int() { GIMSK &= ~(1 << PCIE); }

    inline void setup_ack_detection() {
        PCMSK |= (1 << PCINT4);
        disable_ack_detection_int();
    }

    inline uint8_t roll_and_inverse_data(uint8_t data) {
        return ~((data << 1) | (data >> 7));
    }

    void _release_reset() {
        if ((sync_state == TIMER_READY_HARD_RESET) && user_ready_hard_reset) {
            set_clk_high();
            cli();
            wdt_enable(WDTO_15MS);
            wdt_reset();
            while (true) {
            }
        }
    }

    void release_reset() {
        user_ready_hard_reset = 1;
        _release_reset();
    }

    static void frame_set_dat_bit();
    static void frame_set_clk_low();
    static void frame_set_clk_high();
    static void begin_transfer();
    static void end_transfer();
    static void send_one_bit();
    static void hard_reset();
    static void wait_for_hard_reset();
    static void idle();

    static void begin_transfer() {
        bit_counter = 0;
        set_pins_as_output();
        state = frame_set_dat_bit;
    }

    static void frame_set_dat_bit() {
        set_dat_bit(data & (128 >> bit_counter));
        state = frame_set_clk_low;
    }

    static void frame_set_clk_low() {
        set_clk_low();
        state = frame_set_clk_high;
    }

    static void frame_set_clk_high() {
        set_clk_high();
        state = (++bit_counter < 8) ? frame_set_dat_bit : end_transfer;
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

    static void send_one_bit() {
        disable_timer_int();
        disable_ack_detection_int();
        if (sync_state != UNSYNCED) {
            sync_state = SEND_ONE_BIT;
            data_to_recover = data;
        }
        bit_counter = 7;
        data = 0;
        state = frame_set_dat_bit;
        setup_timer_for_frame();
        set_pins_as_output();
        enable_timer_int();
    }

    static void wait_for_hard_reset() {
        bit_counter++;
        // 40 * 250ms = 10s.
        if (bit_counter > 40) {
            disable_timer_int();
            disable_ack_detection_int();
            hard_reset();
        }
    }

    static void wait_for_500_ms() {
        bit_counter++;
        if (bit_counter > 2) {
            disable_timer_int();
            sync_state = TIMER_READY_HARD_RESET;
            _release_reset();
        }
    }

    static void hard_reset() {
        set_pins_as_output();
        set_clk_low();
        bit_counter = 0;
        state = wait_for_500_ms;
        enable_timer_int();
    }

    static void idle() {}

    ISR(TIM1_OVF_vect) { (*state)(); }

    ISR(PCINT0_vect) {
        const uint8_t lost_sync_code = 0xF9;
        const uint8_t initiate_power_up_stream_code = 0xFD;
        const uint8_t terminate_power_up_stream_code = 0xFE;
        const uint8_t reset_warning = 0x78;
        // on rising edge of the acknowledment period from the Amiga
        if (PINB & pins::amiga::DAT) {
            disable_ack_detection_int();
            disable_timer_int();
            if (sync_state == SEND_ONE_BIT) {
                bit_counter = 0;
                data = roll_and_inverse_data(lost_sync_code);
                sync_state = SEND_LOST_SYNC;
                state = frame_set_dat_bit;
            } else if (sync_state == SEND_LOST_SYNC) {
                bit_counter = 0;
                data = data_to_recover;
                sync_state = SEND_LOST_DATA;
                state = frame_set_dat_bit;
            } else if ((sync_state == TERMINATE_STREAM) || (sync_state == SEND_LOST_DATA)) {
                sync_state = SYNCED;
                state = idle;
                return;
            } else if (sync_state == SYNCED) {
                if (scancodes.read(&data)) {
                    bit_counter = 0;
                    state = frame_set_dat_bit;
                } else {
                    state = idle;
                    return;
                }
            } else if (sync_state == UNSYNCED) {
                bit_counter = 0;
                data = roll_and_inverse_data(initiate_power_up_stream_code);
                sync_state = INITIATE_STREAM;
                state = frame_set_dat_bit;
            } else if (sync_state == INITIATE_STREAM) {
                bit_counter = 0;
                data = roll_and_inverse_data(terminate_power_up_stream_code);
                sync_state = TERMINATE_STREAM;
                state = frame_set_dat_bit;
            } else if (sync_state == FIRST_RESET_WARNING) {
                bit_counter = 0;
                data = roll_and_inverse_data(reset_warning);
                sync_state = SECOND_RESET_WARNING;
                state = frame_set_dat_bit;
            } else if (sync_state == SECOND_RESET_WARNING) {
                hard_reset();
                return;
            }
            setup_timer_for_frame();
            set_pins_as_output();
            enable_timer_int();
        } else if (sync_state == SECOND_RESET_WARNING) {
            // on falling edge of the second reset warning acknowledgement
            disable_ack_detection_int();
            disable_timer_int();
            state = wait_for_hard_reset;
            bit_counter = 0;
            setup_timer_for_hard_reset();
            enable_timer_int();
            enable_ack_detection_int();
        }
    }

    void begin() {
        setup_ack_detection();
        setup_timer();
        setup_timer_for_resync();
        set_pins_as_pull_up();
        DDRB &= ~pins::amiga::RST;
        PORTB &= ~pins::amiga::RST;
        sync_state = UNSYNCED;
        user_ready_hard_reset = 0;
        send_one_bit();
    }

    bool is_ready() { return sync_state == SYNCED; }

    bool send(uint8_t keycode) {
        keycode = roll_and_inverse_data(keycode);
        if (scancodes.write(&keycode)) {
            if (state == idle) {
                scancodes.read(&data);
                state = begin_transfer;
                setup_timer_for_frame();
                enable_timer_int();
            }
            return true;
        } else {
            return false;
        }
    }

    void hold_reset() {
        const uint8_t reset_warning = 0x78;
        // drain the output
        while (state != idle && is_ready()) {
        }
        if (!is_ready()) {
            hard_reset();
            return;
        }
        data = roll_and_inverse_data(reset_warning);
        sync_state = FIRST_RESET_WARNING;
        state = begin_transfer;
        setup_timer_for_frame();
        enable_timer_int();
    }

} // namespace amiga
