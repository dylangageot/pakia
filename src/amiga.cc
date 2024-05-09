#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>

#include "amiga.hh"
#include "circular_buffer.hh"
#include "pins.hh"

namespace amiga {

    struct circular_buffer<uint8_t, 8> scancodes;

    struct fsm {
        void (*state)();
        uint8_t data;
        uint8_t data_to_recover;
        uint8_t bit_counter;
        enum fail_state fail_state;
        enum sync_state sync_state;
        uint8_t user_reset;

        void begin();
        bool is_ready();
        bool trigger_send();
        void hold_reset();
        void release_reset();
    } fsm;

    void begin() { fsm.begin(); }

    bool is_ready() { return fsm.is_ready(); }

    void hold_reset() { fsm.hold_reset(); }

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

    void release_reset() {
        fsm.user_reset = 0;
        fsm.release_reset();
    }

    void fsm::release_reset() {
        if ((user_reset == 0) && (sync_state == RESET)) {
            set_clk_high();
            cli();
            wdt_enable(WDTO_15MS);
            //WDTCR = (1 << WDCE) | (1 << WDE) | WDTO_1S;
            wdt_reset();
            while (true) {
            }
        }
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
        fsm.bit_counter = 0;
        fsm.fail_state = OK;
        set_pins_as_output();
        fsm.state = frame_set_dat_bit;
    }

    static void frame_set_dat_bit() {
        set_dat_bit(fsm.data & (128 >> fsm.bit_counter));
        fsm.state = frame_set_clk_low;
    }

    static void frame_set_clk_low() {
        set_clk_low();
        fsm.state = frame_set_clk_high;
    }

    static void frame_set_clk_high() {
        set_clk_high();
        fsm.state = (++fsm.bit_counter < 8) ? frame_set_dat_bit : end_transfer;
    }

    static void end_transfer() {
        disable_timer_int();
        set_pins_as_pull_up();
        fsm.state = send_one_bit;
        setup_timer_for_resync();
        if (fsm.sync_state == FIRST_RESET_WARNING) {
            fsm.state = hard_reset;
        } else if (fsm.sync_state == SECOND_RESET_WARNING) {
            fsm.state = hard_reset;
            setup_timer_for_hard_reset();
        }
        enable_timer_int();
        enable_ack_detection_int();
    }

    static void send_one_bit() {
        disable_timer_int();
        disable_ack_detection_int();
        // got call? we got out of sync
        if ((fsm.sync_state > UNSYNCED) && (fsm.fail_state == OK)) {
            // Serial.print("Failed to send data... ");
            fsm.fail_state = SEND_ONE_BIT;
            fsm.data_to_recover = fsm.data;
        }
        // Serial.println("Proceed to sending one bit every 143ms");
        //  send a single bit with value 1 (low state)
        fsm.bit_counter = 7;
        fsm.data = 0;
        fsm.state = frame_set_dat_bit;
        setup_timer_for_frame();
        set_pins_as_output();
        enable_timer_int();
    }

    static void wait_for_hard_reset() {
        fsm.bit_counter++;
        // 40 * 250ms = 10s.
        if (fsm.bit_counter > 40) {
            disable_timer_int();
            disable_ack_detection_int();
            hard_reset();
        }
    }

    static void wait_for_500_ms() {
        fsm.bit_counter++;
        if (fsm.bit_counter > 2) {
            fsm.sync_state = RESET;
            disable_timer_int();
            fsm.release_reset();
        }
    }

    static void hard_reset() {
        // what do we do here?
        set_pins_as_output();
        set_clk_low();
        fsm.bit_counter = 0;
        fsm.state = wait_for_500_ms;
        enable_timer_int();
    }

    static void idle() {}

    ISR(TIM1_OVF_vect) { (*fsm.state)(); }

    ISR(PCINT0_vect) {
        const uint8_t lost_sync_code = 0xF9;
        const uint8_t initiate_power_up_stream_code = 0xFD;
        const uint8_t terminate_power_up_stream_code = 0xFE;
        const uint8_t reset_warning = 0x78;
        // on rising edge
        if (PINB & pins::amiga::DAT) {
            disable_ack_detection_int();
            disable_timer_int();
            if (fsm.state == send_one_bit) {
                if (fsm.fail_state > OK) {
                    if (fsm.fail_state == SEND_ONE_BIT) {
                        // Serial.println("One bit frame got ACKed, send a lost
                        // sync code");
                        fsm.bit_counter = 0;
                        fsm.data = roll_and_inverse_data(lost_sync_code);
                        fsm.fail_state = SEND_LOST_SYNC;
                        fsm.state = frame_set_dat_bit;
                    } else if (fsm.fail_state == SEND_LOST_SYNC) {
                        // else resend data that failed to be send
                        // Serial.println("Lost sync code got ACKed, send the
                        // original code");
                        fsm.bit_counter = 0;
                        fsm.data = fsm.data_to_recover;
                        fsm.fail_state = OK;
                        fsm.state = frame_set_dat_bit;
                    }
                } else if (fsm.sync_state == TERMINATE_STREAM) {
                    // still data to send?
                    if (scancodes.read(&fsm.data)) {
                        fsm.bit_counter = 0;
                        fsm.state = frame_set_dat_bit;
                    } else {
                        fsm.state = idle;
                        return;
                    }
                } else if (fsm.sync_state == UNSYNCED) {
                    // Serial.println("One bit frame got ACKed, send a initiate
                    // power up stream code");
                    fsm.bit_counter = 0;
                    fsm.data =
                        roll_and_inverse_data(initiate_power_up_stream_code);
                    fsm.sync_state = INITIATE_STREAM;
                    fsm.state = frame_set_dat_bit;
                } else if (fsm.sync_state == INITIATE_STREAM) {
                    // Serial.println("Initiate power up stream frame got ACKed,
                    // send a terminate stream code");
                    fsm.bit_counter = 0;
                    fsm.data =
                        roll_and_inverse_data(terminate_power_up_stream_code);
                    fsm.sync_state = TERMINATE_STREAM;
                    fsm.state = frame_set_dat_bit;
                }
            } else if (fsm.sync_state == FIRST_RESET_WARNING) {
                fsm.bit_counter = 0;
                fsm.data = roll_and_inverse_data(reset_warning);
                fsm.sync_state = SECOND_RESET_WARNING;
                fsm.state = frame_set_dat_bit;
            } else if (fsm.sync_state == SECOND_RESET_WARNING) {
                hard_reset();
                return;
            }
            setup_timer_for_frame();
            set_pins_as_output();
            enable_timer_int();
        } else if (fsm.sync_state == SECOND_RESET_WARNING) {
            // on falling edge
            disable_ack_detection_int();
            disable_timer_int();
            fsm.state = wait_for_hard_reset;
            fsm.bit_counter = 0;
            setup_timer_for_hard_reset();
            enable_timer_int();
            enable_ack_detection_int();
        }
    }

    void fsm::begin() {
        setup_ack_detection();
        setup_timer();
        setup_timer_for_resync();
        set_pins_as_pull_up();
        DDRB &= ~pins::amiga::RST;
        PORTB &= ~pins::amiga::RST;
        fail_state = OK;
        sync_state = UNSYNCED;
        user_reset = 0;
        send_one_bit();
    }

    bool fsm::is_ready() {
        return (sync_state == TERMINATE_STREAM) && (fail_state == OK);
    }

    bool fsm::trigger_send() {
        if (state == idle) {
            scancodes.read(&data);
            state = begin_transfer;
            setup_timer_for_frame();
            enable_timer_int();
        }
        return true;
    }

    bool send(uint8_t keycode) {
        keycode = roll_and_inverse_data(keycode);
        if (scancodes.write(&keycode)) {
            return fsm.trigger_send();
        } else {
            return false;
        }
    }

    void fsm::hold_reset() {
        const uint8_t reset_warning = 0x78;
        user_reset = 1;
        // drain the output
        while (state != idle && is_ready()) {
        }
        if (!is_ready()) {
            // unexpected issue, go hard reset and reset after keys released
        }
        data = roll_and_inverse_data(reset_warning);
        sync_state = FIRST_RESET_WARNING;
        state = begin_transfer;
        setup_timer_for_frame();
        enable_timer_int();
    }

} // namespace amiga
