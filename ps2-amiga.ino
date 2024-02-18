#define __AVR_ATmega328P__
#include <avr/io.h>

#include "ps2.hh"

const size_t AMIGA_CLK_PIN = 2;
const size_t AMIGA_DAT_PIN = 5;

volatile uint8_t amiga_data;
volatile uint8_t amiga_bit_counter;
void (*volatile amiga_state)();

inline void amiga_setup_timer()
{
    // TCCR1A = (1 << COM1A0);
    TCCR1B = (1 << WGM12) | (1 << CS10); // | (1 << CS10);
    OCR1AH = 0x01;
    OCR1AL = 0x80;
    TCNT1H = 0;
    TCNT1L = 0;
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

inline void amiga_set_bit(uint8_t bit)
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
    EIMSK |= (1 << INT0);
}

inline void amiga_disable_ack_detection_int()
{
    EIMSK ^= (1 << INT0);
}

inline void amiga_setup_ack_detection()
{
    EICRA = (EICRA & ~((1 << ISC00) | (1 << ISC01))) | (FALLING << ISC00);
    amiga_disable_ack_detection_int();
}

void setup()
{
    Serial.begin(115200);
    ps2_fsm.begin();

    amiga_state = amiga_idle;
    amiga_setup_timer();
    amiga_setup_ack_detection();
    amiga_set_pins_as_pull_up();
}

void amiga_idle() {}

void amiga_wait_for_ack() {
};

void amiga_end_transfer()
{
    amiga_set_pins_as_pull_up();
    TIMSK1 &= ~(1 << OCIE1A);

    //
    amiga_state = amiga_idle;
}

void amiga_begin_transfer()
{
    amiga_bit_counter = 0;
    amiga_set_pins_as_output();
    amiga_state = amiga_frame_set_dat_bit;
}

void amiga_frame_set_dat_bit()
{
    amiga_set_bit(amiga_data & (128 >> amiga_bit_counter));
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

void send_data_to_amiga(uint8_t data)
{
    char s[80];
    if (amiga_state == amiga_idle)
    {
        // roll out data and reverse bit
        amiga_data = ~((data << 1) | (data >> 7));
        amiga_state = amiga_begin_transfer;
        TIMSK1 = (1 << OCIE1A);
    }
    else
    {
        Serial.println("Too quick, amiga sender is busy");
    }
}

ISR(TIMER1_COMPA_vect)
{
    (*amiga_state)();
}

ISR(INT0_vect)
{
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