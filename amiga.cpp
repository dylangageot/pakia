#define __AVR_ATmega328P__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <Arduino.h>

#include "amiga.hh"

namespace amiga
{

    struct fsm fsm;

    void begin() {
        fsm.begin();
    }

    bool send(uint8_t keycode) {
        return fsm.send(keycode);
    }

    inline void setup_timer()
    {
        TCCR1A = (1 << COM1A0);
    }

    inline void setup_timer_for_frame()
    {
        TCCR1B = (1 << WGM12) | (1 << CS10); // | (1 << CS10);
        OCR1AH = 0x01;
        OCR1AL = 0x80;
        TCNT1H = 0;
        TCNT1L = 0;
    }

    inline void setup_timer_for_resync()
    {
        TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);
        OCR1AH = 0x8B;
        OCR1AL = 0xA6;
        TCNT1H = 0;
        TCNT1L = 0;
    }

    inline void enable_timer_int()
    {
        TIFR1 &= ~(1 << OCIE1A);
        TIMSK1 |= (1 << OCIE1A);
    }

    inline void disable_timer_int()
    {
        TIMSK1 &= ~(1 << OCIE1A);
    }

    inline void set_pins_as_pull_up()
    {
        DDRD &= ~((1 << CLK_PIN) | (1 << DAT_PIN));
        PORTD |= (1 << CLK_PIN) | (1 << DAT_PIN);
    }

    inline void set_pins_as_output()
    {
        DDRD |= (1 << CLK_PIN) | (1 << DAT_PIN);
    }

    inline void set_dat_bit(uint8_t bit)
    {
        PORTD = PORTD & ~(1 << DAT_PIN) | ((bit) ? (1 << DAT_PIN) : 0);
    }

    inline void set_clk_high()
    {
        PORTD |= (1 << CLK_PIN);
    }

    inline void set_clk_low()
    {
        PORTD &= ~(1 << CLK_PIN);
    }

    inline void enable_ack_detection_int()
    {
        EIFR |= (1 << INTF0);
        EIMSK |= (1 << INT0);
    }

    inline void disable_ack_detection_int()
    {
        EIMSK &= ~(1 << INT0);
    }

    inline void setup_ack_detection()
    {
        EICRA = (EICRA & ~((1 << ISC00) | (1 << ISC01))) | (RISING << ISC00);
    }

    inline uint8_t roll_and_inverse_data(uint8_t data)
    {
        return ~((data << 1) | (data >> 7));
    }

    static void frame_set_dat_bit();
    static void frame_set_clk_low();
    static void frame_set_clk_high();
    static void begin_transfer();
    static void end_transfer();
    static void send_one_bit();
    static void idle();

    static void begin_transfer()
    {
        fsm.bit_counter = 0;
        fsm.fail_state = OK;
        set_pins_as_output();
        fsm.state = frame_set_dat_bit;
    }

    static void frame_set_dat_bit()
    {
        set_dat_bit(fsm.data & (128 >> fsm.bit_counter));
        fsm.state = frame_set_clk_low;
    }

    static void frame_set_clk_low()
    {
        set_clk_low();
        fsm.state = frame_set_clk_high;
    }

    static void frame_set_clk_high()
    {
        set_clk_high();
        fsm.state = (++fsm.bit_counter < 8) ? frame_set_dat_bit : end_transfer;
    }

    static void end_transfer()
    {
        disable_timer_int();
        set_pins_as_pull_up();
        fsm.state = send_one_bit;
        setup_timer_for_resync();
        enable_timer_int();
        enable_ack_detection_int();
    }

    static void send_one_bit()
    {
        disable_timer_int();
        disable_ack_detection_int();
        // got call? we got out of sync
        if ((fsm.sync_state > UNSYNCED) && (fsm.fail_state == OK))
        {
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

    ISR(TIMER1_COMPA_vect)
    {
        (*fsm.state)();
    }

    ISR(INT0_vect)
    {
        const uint8_t lost_sync_code = 0xF9;
        const uint8_t initiate_power_up_stream_code = 0xFD;
        const uint8_t terminate_power_up_stream_code = 0xFE;
        PORTB |= (1 << PIN0);
        disable_ack_detection_int();
        disable_timer_int();
        if (fsm.state == send_one_bit)
        {
            if (fsm.fail_state > OK)
            {
                if (fsm.fail_state == SEND_ONE_BIT)
                {
                    // Serial.println("One bit frame got ACKed, send a lost sync code");
                    fsm.bit_counter = 0;
                    fsm.data = roll_and_inverse_data(lost_sync_code);
                    fsm.fail_state = SEND_LOST_SYNC;
                    fsm.state = frame_set_dat_bit;
                }
                else if (fsm.fail_state == SEND_LOST_SYNC)
                {
                    // else resend data that failed to be send
                    // Serial.println("Lost sync code got ACKed, send the original code");
                    fsm.bit_counter = 0;
                    fsm.data = fsm.data_to_recover;
                    fsm.fail_state = OK;
                    fsm.state = frame_set_dat_bit;
                }
            }
            else if (fsm.sync_state == TERMINATE_STREAM)
            {
                // Serial.println("Key ACKed, idling");
                fsm.state = idle;
                PORTB &= ~(1 << PIN0);
                return;
            }
            else if (fsm.sync_state == UNSYNCED)
            {
                // Serial.println("One bit frame got ACKed, send a initiate power up stream code");
                fsm.bit_counter = 0;
                fsm.data = roll_and_inverse_data(initiate_power_up_stream_code);
                fsm.sync_state = INITIATE_STREAM;
                fsm.state = frame_set_dat_bit;
            }
            else if (fsm.sync_state == INITIATE_STREAM)
            {
                // Serial.println("Initiate power up stream frame got ACKed, send a terminate stream code");
                fsm.bit_counter = 0;
                fsm.data = roll_and_inverse_data(terminate_power_up_stream_code);
                fsm.sync_state = TERMINATE_STREAM;
                fsm.state = frame_set_dat_bit;
            }
            setup_timer_for_frame();
            set_pins_as_output();
            enable_timer_int();
            PORTB &= ~(1 << PIN0);
        }
    }

    void fsm::begin()
    {
        // int debug pin
        PORTB &= ~(1 << PIN0);
        DDRB |= (1 << PIN0);
        setup_ack_detection();
        setup_timer();
        setup_timer_for_resync();
        set_pins_as_pull_up();
        fail_state = OK;
        sync_state = UNSYNCED;
        send_one_bit();
    }

    bool fsm::send(uint8_t keycode)
    {
        if (state != idle)
        {
            return false;
        }
        // roll out data and reverse bit
        data = roll_and_inverse_data(keycode);
        state = begin_transfer;
        setup_timer_for_frame();
        enable_timer_int();
        return true;
    }

}
