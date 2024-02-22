#define __AVR_ATmega328P__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <Arduino.h>

#include "amiga.hh"

struct amiga_fsm amiga_fsm;

inline void amiga_setup_timer()
{
    TCCR1A = (1 << COM1A0);
}

inline void amiga_setup_timer_for_frame()
{
    TCCR1B = (1 << WGM12) | (1 << CS10); // | (1 << CS10);
    OCR1AH = 0x01;
    OCR1AL = 0x80;
    TCNT1H = 0;
    TCNT1L = 0;
}

inline void amiga_setup_timer_for_resync()
{
    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);
    OCR1AH = 0x8B;
    OCR1AL = 0xA6;
    TCNT1H = 0;
    TCNT1L = 0;
}

inline void amiga_enable_timer_int()
{
    TIFR1 &= ~(1 << OCIE1A);
    TIMSK1 |= (1 << OCIE1A);
}

inline void amiga_disable_timer_int()
{
    TIMSK1 &= ~(1 << OCIE1A);
}

inline void amiga_set_pins_as_pull_up()
{
    DDRD &= ~((1 << AMIGA_CLK_PIN) | (1 << AMIGA_DAT_PIN));
    PORTD |= (1 << AMIGA_CLK_PIN) | (1 << AMIGA_DAT_PIN);
}

inline void amiga_set_pins_as_output()
{
    DDRD |= (1 << AMIGA_CLK_PIN) | (1 << AMIGA_DAT_PIN);
}

inline void amiga_set_dat_bit(uint8_t bit)
{
    PORTD = PORTD & ~(1 << AMIGA_DAT_PIN) | ((bit) ? (1 << AMIGA_DAT_PIN) : 0);
}

inline void amiga_set_clk_high()
{
    PORTD |= (1 << AMIGA_CLK_PIN);
}

inline void amiga_set_clk_low()
{
    PORTD &= ~(1 << AMIGA_CLK_PIN);
}

inline void amiga_enable_ack_detection_int()
{
    EIFR |= (1 << INTF0);
    EIMSK |= (1 << INT0);
}

inline void amiga_disable_ack_detection_int()
{
    EIMSK &= ~(1 << INT0);
}

inline void amiga_setup_ack_detection()
{
    EICRA = (EICRA & ~((1 << ISC00) | (1 << ISC01))) | (RISING << ISC00);
}

inline uint8_t amiga_roll_and_inverse_data(uint8_t data) {
    return ~((data << 1) | (data >> 7));
}

static void amiga_frame_set_dat_bit();
static void amiga_frame_set_clk_low();
static void amiga_frame_set_clk_high();
static void amiga_begin_transfer();
static void amiga_end_transfer();
static void amiga_send_one_bit();
static void amiga_idle();

static void amiga_begin_transfer()
{
    amiga_fsm.bit_counter = 0;
    amiga_fsm.fail_state = OK;
    amiga_set_pins_as_output();
    amiga_fsm.state = amiga_frame_set_dat_bit;
}

static void amiga_frame_set_dat_bit()
{
    amiga_set_dat_bit(amiga_fsm.data & (128 >> amiga_fsm.bit_counter));
    amiga_fsm.state = amiga_frame_set_clk_low;
}

static void amiga_frame_set_clk_low()
{
    amiga_set_clk_low();
    amiga_fsm.state = amiga_frame_set_clk_high;
}

static void amiga_frame_set_clk_high()
{
    amiga_set_clk_high();
    amiga_fsm.state = (++amiga_fsm.bit_counter < 8) ? amiga_frame_set_dat_bit : amiga_end_transfer;
}

static void amiga_end_transfer()
{
    amiga_disable_timer_int();
    amiga_set_pins_as_pull_up();    
    amiga_fsm.state = amiga_send_one_bit;
    amiga_setup_timer_for_resync();
    amiga_enable_timer_int();
    amiga_enable_ack_detection_int();
}

static void amiga_send_one_bit()
{
    amiga_disable_timer_int();
    amiga_disable_ack_detection_int();
    // got call? we got out of sync
    if ((amiga_fsm.sync_state > UNSYNCED) && (amiga_fsm.fail_state == OK))
    {
        //Serial.print("Failed to send data... ");
        amiga_fsm.fail_state = SEND_ONE_BIT;
        amiga_fsm.data_to_recover = amiga_fsm.data;
    }
    //Serial.println("Proceed to sending one bit every 143ms");
    // send a single bit with value 1 (low state)
    amiga_fsm.bit_counter = 7;
    amiga_fsm.data = 0;
    amiga_fsm.state = amiga_frame_set_dat_bit;
    amiga_setup_timer_for_frame();
    amiga_set_pins_as_output();
    amiga_enable_timer_int();
}

static void amiga_idle() {}

ISR(TIMER1_COMPA_vect)
{
    (*amiga_fsm.state)();
}

ISR(INT0_vect)
{
    const uint8_t lost_sync_code = 0xF9;
    const uint8_t initiate_power_up_stream_code = 0xFD;
    const uint8_t terminate_power_up_stream_code = 0xFE;
    PORTB |= (1 << PIN0);
    amiga_disable_ack_detection_int();
    amiga_disable_timer_int();
    if (amiga_fsm.state == amiga_send_one_bit)
    {
        if (amiga_fsm.fail_state > OK)
        {
            if (amiga_fsm.fail_state == SEND_ONE_BIT)
            {
                //Serial.println("One bit frame got ACKed, send a lost sync code");
                amiga_fsm.bit_counter = 0;
                amiga_fsm.data = amiga_roll_and_inverse_data(lost_sync_code);
                amiga_fsm.fail_state = SEND_LOST_SYNC;
                amiga_fsm.state = amiga_frame_set_dat_bit;
            }
            else if (amiga_fsm.fail_state == SEND_LOST_SYNC)
            {
                // else resend data that failed to be send
                //Serial.println("Lost sync code got ACKed, send the original code");
                amiga_fsm.bit_counter = 0;
                amiga_fsm.data = amiga_fsm.data_to_recover;
                amiga_fsm.fail_state = OK;
                amiga_fsm.state = amiga_frame_set_dat_bit;
            }
        }
        else if (amiga_fsm.sync_state == TERMINATE_STREAM)
        {
            //Serial.println("Key ACKed, idling");
            amiga_fsm.state = amiga_idle;
            PORTB &= ~(1 << PIN0);
            return;
        }
        else if (amiga_fsm.sync_state == UNSYNCED)
        {
            //Serial.println("One bit frame got ACKed, send a initiate power up stream code");
            amiga_fsm.bit_counter = 0;
            amiga_fsm.data = amiga_roll_and_inverse_data(initiate_power_up_stream_code);
            amiga_fsm.sync_state = INITIATE_STREAM;
            amiga_fsm.state = amiga_frame_set_dat_bit;
        }
        else if (amiga_fsm.sync_state == INITIATE_STREAM)
        {
            //Serial.println("Initiate power up stream frame got ACKed, send a terminate stream code");
            amiga_fsm.bit_counter = 0;
            amiga_fsm.data = amiga_roll_and_inverse_data(terminate_power_up_stream_code);
            amiga_fsm.sync_state = TERMINATE_STREAM;
            amiga_fsm.state = amiga_frame_set_dat_bit;
        }
        amiga_setup_timer_for_frame();
        amiga_set_pins_as_output();
        amiga_enable_timer_int();
        PORTB &= ~(1 << PIN0);
    }
}

void amiga_fsm::begin() {
    // int debug pin
    PORTB &= ~(1 << PIN0);
    DDRB |= (1 << PIN0);
    amiga_setup_ack_detection();
    amiga_setup_timer();
    amiga_setup_timer_for_resync();
    amiga_set_pins_as_pull_up();
    fail_state = OK;
    sync_state = UNSYNCED;
    amiga_send_one_bit();
}

bool amiga_fsm::send_keycode(uint8_t keycode)
{
    if (state != amiga_idle) {
        return false;
    }
    // roll out data and reverse bit
    data = amiga_roll_and_inverse_data(keycode);
    state = amiga_begin_transfer;
    amiga_setup_timer_for_frame();
    amiga_enable_timer_int();
    return true;
}
