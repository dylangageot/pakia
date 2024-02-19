#define __AVR_ATmega328P__
#include <avr/io.h>

#include "ps2.hh"

const size_t AMIGA_CLK_PIN = 5;
const size_t AMIGA_DAT_PIN = 2;

enum amiga_fail_state
{
    OK = 0,
    SEND_ONE_BIT,
    SEND_LOST_SYNC
};

enum amiga_sync_state
{
    UNSYNCED = 0,
    INITIATE_STREAM,
    TERMINATE_STREAM
};

volatile uint8_t amiga_data;
volatile uint8_t amiga_bit_counter;
volatile enum amiga_fail_state amiga_fail_state;
volatile enum amiga_sync_state amiga_sync_state;
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
    TIFR1 &= ~(1 << OCIE1A);
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

void setup()
{
    Serial.begin(115200);
    ps2_fsm.begin();

    PORTB &= ~(1 << PIN0);
    DDRB |= (1 << PIN0);

    amiga_setup_ack_detection();
    amiga_setup_timer();
    amiga_setup_timer_for_resync();
    amiga_set_pins_as_pull_up();
    amiga_fail_state = OK;
    amiga_sync_state = UNSYNCED;
    amiga_send_one_bit();
}

void amiga_send_one_bit()
{
    amiga_disable_timer_int();
    amiga_disable_ack_detection_int();
    // got call? we got out of sync
    if ((amiga_sync_state > UNSYNCED) && (amiga_fail_state == OK))
    {
        //Serial.print("Failed to send data... ");
        amiga_fail_state = SEND_ONE_BIT;
        amiga_data_to_recover = amiga_data;
    }
    //Serial.println("Proceed to sending one bit every 143ms");
    // send a single bit with value 1 (low state)
    amiga_bit_counter = 7;
    amiga_data = 0;
    amiga_state = amiga_frame_set_dat_bit;
    amiga_setup_timer_for_frame();
    amiga_set_pins_as_output();
    amiga_enable_timer_int();
}

void amiga_end_transfer()
{
    amiga_disable_timer_int();
    amiga_set_pins_as_pull_up();    
    amiga_state = amiga_send_one_bit;
    amiga_setup_timer_for_resync();
    amiga_enable_timer_int();
    amiga_enable_ack_detection_int();
}

void amiga_begin_transfer()
{
    amiga_bit_counter = 0;
    amiga_fail_state = OK;
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

void amiga_idle() {}

ISR(TIMER1_COMPA_vect)
{
    (*amiga_state)();
}

ISR(INT0_vect)
{
    const uint8_t lost_sync_code = 0xF9;
    const uint8_t initiate_power_up_stream_code = 0xFD;
    const uint8_t terminate_key_stream_power_up_stream_code = 0xFE;
    PORTB |= (1 << PIN0);
    amiga_disable_ack_detection_int();
    amiga_disable_timer_int();
    if (amiga_state == amiga_send_one_bit)
    {
        if (amiga_fail_state > OK)
        {
            if (amiga_fail_state == SEND_ONE_BIT)
            {
                //Serial.println("One bit frame got ACKed, send a lost sync code");
                amiga_bit_counter = 0;
                amiga_data = ~((lost_sync_code << 1) | (lost_sync_code >> 7));
                amiga_fail_state = SEND_LOST_SYNC;
                amiga_state = amiga_frame_set_dat_bit;
            }
            else if (amiga_fail_state == SEND_LOST_SYNC)
            {
                // else resend data that failed to be send
                //Serial.println("Lost sync code got ACKed, send the original code");
                amiga_bit_counter = 0;
                amiga_data = amiga_data_to_recover;
                amiga_fail_state = OK;
                amiga_state = amiga_frame_set_dat_bit;
            }
        }
        else if (amiga_sync_state == TERMINATE_STREAM)
        {
            //Serial.println("Key ACKed, idling");
            amiga_state = amiga_idle;
            PORTB &= ~(1 << PIN0);
            return;
        }
        else if (amiga_sync_state == UNSYNCED)
        {
            //Serial.println("One bit frame got ACKed, send a initiate power up stream code");
            amiga_bit_counter = 0;
            amiga_data = ~((initiate_power_up_stream_code << 1) | (initiate_power_up_stream_code >> 7));
            amiga_sync_state = INITIATE_STREAM;
            amiga_state = amiga_frame_set_dat_bit;
        }
        else if (amiga_sync_state == INITIATE_STREAM)
        {
            //Serial.println("Initiate power up stream frame got ACKed, send a terminate stream code");
            amiga_bit_counter = 0;
            amiga_data = ~((terminate_key_stream_power_up_stream_code << 1) | (terminate_key_stream_power_up_stream_code >> 7));
            amiga_sync_state = TERMINATE_STREAM;
            amiga_state = amiga_frame_set_dat_bit;
        }
        amiga_setup_timer_for_frame();
        amiga_set_pins_as_output();
        amiga_enable_timer_int();
        PORTB &= ~(1 << PIN0);
    }
}

void send_data_to_amiga(uint8_t data)
{
    static uint8_t push_down = 0;
    char s[80];
    if (amiga_state == amiga_idle)
    {
        data |= (push_down & 0x80);
        push_down ^= 0x80;
        sprintf(s, "Sending key: 0x%x\n", data);
        Serial.println(s);
        // roll out data and reverse bit
        amiga_data = ~((data << 1) | (data >> 7));
        amiga_state = amiga_begin_transfer;
        amiga_setup_timer_for_frame();
        amiga_enable_timer_int();
    }
    else
    {
        Serial.println("Amiga sender is busy");
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
        send_data_to_amiga(0x35);
        sprintf(s, "Received key: 0x%x\n", key);
        Serial.println(s);
    }
}