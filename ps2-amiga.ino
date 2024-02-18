#define __AVR_ATmega328P__
#include <avr/io.h>

#include "ps2.hh"

const size_t AMIGA_CLK_PIN = 5;
const size_t AMIGA_DAT_PIN = 2;

volatile uint8_t amiga_data;
volatile uint8_t amiga_bit_counter;
volatile bool amiga_failed;
volatile uint8_t amiga_data_to_recover;
void (*volatile amiga_state)();

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
    TIMSK1 |= (1 << OCIE1A);
}

inline void amiga_disable_timer_int()
{
    TIMSK1 &= ~(1 << OCIE1A);
}

inline void amiga_set_pins_as_pull_up()
{
    DDRD &= ~((1 << AMIGA_CLK_PIN) | (1 << AMIGA_DAT_PIN)); // pin D4 as output, toggle by ISR
    PORTD |= (1 << AMIGA_CLK_PIN) | (1 << AMIGA_DAT_PIN);   // pull up!
}

inline bool amiga_frame_is_not_ack()
{
    return PORTD & (1 << AMIGA_DAT_PIN);
}

inline void amiga_set_pins_as_output()
{
    DDRD |= (1 << AMIGA_CLK_PIN) | (1 << AMIGA_DAT_PIN);
}

inline void amiga_set_dat_bit(uint8_t bit)
{
    PORTD = PORTD & ~(1 << AMIGA_DAT_PIN) | (1 << AMIGA_CLK_PIN) | ((bit) ? (1 << AMIGA_DAT_PIN) : 0);
}

inline void amiga_set_clk_high()
{
    PORTD &= ~(1 << AMIGA_CLK_PIN);
}

inline void amiga_set_clk_low()
{
    PORTD |= (1 << AMIGA_CLK_PIN);
}

inline void amiga_enable_ack_detection_int()
{
    EIFR &= (1<< INTF0);
    EIMSK |= (1 << INT0);
}

inline void amiga_disable_ack_detection_int()
{
    EIMSK ^= (1 << INT0);
}

inline void amiga_setup_ack_detection()
{
    EICRA = (EICRA & ~((1 << ISC00) | (1 << ISC01))) | (FALLING << ISC00);
}

void setup()
{
    Serial.begin(115200);
    ps2_fsm.begin();

    amiga_state = amiga_idle;
    amiga_disable_timer_int();
    amiga_setup_timer();
    amiga_disable_ack_detection_int();
    amiga_setup_ack_detection();
    amiga_set_pins_as_pull_up();
}

void amiga_idle() {}

void amiga_wait_for_ack()
{
    amiga_disable_timer_int();
    amiga_disable_ack_detection_int();
    // got call? we got out of sync
    if (!amiga_failed)
    {
        amiga_failed = true;
        amiga_data_to_recover = amiga_data;
    }
    amiga_set_pins_as_output();
    // send a single bit with value 1 (low state)
    amiga_bit_counter = 7;
    amiga_data = 0;
    amiga_state = amiga_frame_set_dat_bit;
    amiga_setup_timer_for_frame();
    amiga_enable_timer_int();
}

void amiga_end_transfer()
{
    amiga_disable_timer_int();
    amiga_set_pins_as_pull_up();
    amiga_state = amiga_wait_for_ack;
    amiga_setup_timer_for_resync();
    amiga_enable_timer_int();
    amiga_enable_ack_detection_int();
}

void amiga_begin_transfer()
{
    amiga_bit_counter = 0;
    amiga_failed = false;
    amiga_set_pins_as_output();
    amiga_state = amiga_frame_set_dat_bit;
}

void amiga_frame_set_dat_bit()
{
    amiga_set_dat_bit(amiga_data & (128 >> amiga_bit_counter));
    amiga_state = amiga_frame_set_clk_low;
}

void amiga_frame_set_clk_low()
{
    amiga_set_clk_low();
    amiga_state = amiga_frame_set_clk_high;
}

void amiga_frame_set_clk_high()
{
    amiga_set_clk_high();
    amiga_state = (++amiga_bit_counter < 8) ? amiga_frame_set_dat_bit : amiga_end_transfer;
}

ISR(TIMER1_COMPA_vect)
{
    (*amiga_state)();
}

ISR(INT0_vect)
{
    amiga_disable_ack_detection_int();
    amiga_disable_timer_int();
    if (amiga_state == amiga_wait_for_ack)
    {
        if (amiga_failed)
        {
            const uint8_t lost_sync_code = 0xF9;
            // if lost sync not sent, send it
            if (amiga_data != lost_sync_code)
            {
                amiga_bit_counter = 0;
                amiga_data = ~((lost_sync_code << 1) | (lost_sync_code >> 7));
                amiga_state = amiga_frame_set_dat_bit;
            }
            else
            {
                // else resend data that failed to be sent
                amiga_failed = false;
                amiga_bit_counter = 0;
                amiga_data = amiga_data_to_recover;
                amiga_state = amiga_frame_set_dat_bit;
            }
            amiga_set_pins_as_output();
            amiga_setup_timer_for_frame();
            amiga_enable_timer_int();
        }
        else
        {
            amiga_state = amiga_idle;
        }
    }
}

void send_data_to_amiga(uint8_t data)
{
    char s[80];
    if (amiga_state == amiga_idle)
    {
        // roll out data and reverse bit
        amiga_data = ~((data << 1) | (data >> 7));
        amiga_state = amiga_begin_transfer;
        amiga_setup_timer_for_frame();
        amiga_enable_timer_int();
    }
    else
    {
        Serial.println("Too quick, amiga sender is busy");
    }
}

void loop()
{
    uint8_t frame_index = 0;
    char s[80];
    volatile struct ps2_frame *frame = (ps2_frames + (ps2_fsm.frame_buffer_index++ & (PS2_FRAME_COUNT_POW_2 - 1)));
    if (frame->available)
    {
        uint8_t key = frame->key;
        frame->available = false;
        frame_index++;
        send_data_to_amiga(key);
        sprintf(s, "Received key: 0x%x\n", key);
        Serial.println(s);
    }
}