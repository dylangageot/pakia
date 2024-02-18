#define __AVR_ATmega328P__
#include <avr/io.h>

#include "ps2.hh"

const size_t AMIGA_CLK_PIN = 0; //8
const size_t AMIGA_DAT_PIN = 1; //9

volatile uint8_t amiga_data;
volatile uint8_t amiga_bit_counter;
void (*volatile amiga_state)();

void setup()
{
    Serial.begin(115200);
    ps2_fsm.begin();

    amiga_state = amiga_idle;

    // timer did it!
    //TCCR1A = (1 << COM1A0);
    TCCR1B = (1 << WGM12) | (1 << CS10); // | (1 << CS10);
    OCR1AH = 0x01;
    OCR1AL = 0x80;
    TCNT1H = 0;
    TCNT1L = 0;

    DDRB &= ~((1 << AMIGA_CLK_PIN) | (1 << AMIGA_DAT_PIN)); // pin D4 as output, toggle by ISR
    PORTB |= (1 << AMIGA_CLK_PIN) | (1 << AMIGA_DAT_PIN);   // pull up!
}

void amiga_idle() {}

void amiga_transfer_end()
{
    DDRB &= ~((1 << AMIGA_CLK_PIN) | (1 << AMIGA_DAT_PIN)); // pin D4 as output, toggle by ISR
    PORTB |= (1 << AMIGA_CLK_PIN) | (1 << AMIGA_DAT_PIN);   // pull up!
    TIMSK1 &= ~(1 << OCIE1A);
    amiga_state = amiga_idle;
}

void amiga_transfer_begin() {
    amiga_bit_counter = 0;
    amiga_state = amiga_position_bit;
}

void amiga_position_bit()
{
    PORTB = PORTB & ~(1 << AMIGA_DAT_PIN) | (1 << AMIGA_CLK_PIN) | ((amiga_data & (128 >> amiga_bit_counter)) ? (1 << AMIGA_DAT_PIN) : 0);
    DDRB |= (1 << AMIGA_CLK_PIN) | (1 << AMIGA_DAT_PIN);
    amiga_state = amiga_set_clk_low;
}

void amiga_set_clk_low()
{
    PORTB &= ~(1 << AMIGA_CLK_PIN);
    amiga_state = amiga_set_clk_high;
}

void amiga_set_clk_high()
{
    PORTB |= (1 << AMIGA_CLK_PIN);
    amiga_state = (++amiga_bit_counter < 8) ? amiga_position_bit : amiga_transfer_end;
}

void send_data_to_amiga(uint8_t data)
{
    char s[80];
    if (amiga_state == amiga_idle)
    {
        // roll out data and reverse bit
        amiga_data = ~((data << 1) | (data >> 7));
        amiga_state = amiga_transfer_begin;
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