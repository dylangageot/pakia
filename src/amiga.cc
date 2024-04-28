#include <avr/interrupt.h>
#include <avr/io.h>

#include "amiga.hh"
#include "pins.hh"

namespace amiga {
    struct fsm fsm;

    void begin() { fsm.begin(); }

    bool send(uint8_t keycode) { return fsm.send(keycode); }

    bool is_ready() { return fsm.is_ready(); }

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
        TCCR1 =
            (TCCR1 & ~((1 << CS13) | (1 << CS12) | (1 << CS11) | (1 << CS10))) |
            (1 << CS13) | (1 << CS12) | (1 << CS11);
        OCR1C = 138;
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

    static void frame_set_dat_bit();
    static void frame_set_clk_low();
    static void frame_set_clk_high();
    static void begin_transfer();
    static void end_transfer();
    static void send_one_bit();
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

    static void idle() {}

    ISR(TIM1_OVF_vect) { (*fsm.state)(); }

    ISR(PCINT0_vect) {
        const uint8_t lost_sync_code = 0xF9;
        const uint8_t initiate_power_up_stream_code = 0xFD;
        const uint8_t terminate_power_up_stream_code = 0xFE;
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
                    // Serial.println("Key ACKed, idling");
                    fsm.state = idle;
                    return;
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
                setup_timer_for_frame();
                set_pins_as_output();
                enable_timer_int();
            }
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
        send_one_bit();
    }

    bool fsm::is_ready() {
        return (sync_state == TERMINATE_STREAM) &&
               (fail_state == OK);
    }

    bool fsm::send(uint8_t keycode) {
        if (!is_ready()) {
            return false;
        }
        // roll out data and reverse bit
        data = roll_and_inverse_data(keycode);
        state = begin_transfer;
        setup_timer_for_frame();
        enable_timer_int();
        return true;
    }
} // namespace amiga
