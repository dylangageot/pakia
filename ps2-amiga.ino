#include <avr/io.h>
#include <avr/interrupt.h>

const size_t PS2_CLK_PIN = 3;
const size_t PS2_DAT_PIN = 4;

const uint8_t PS2_IDLE = 255;

struct ps2_buffer
{
    uint8_t buffer;
    uint8_t counter;
};

static volatile struct ps2_buffer ps2_state;
static volatile struct ps2_buffer ps2_buffered;
static void (*volatile ps2_isr_func)(void);

void ps2_stop_bit()
{
    // save data
    ps2_buffered.counter = ps2_state.counter;
    ps2_buffered.buffer = ps2_state.buffer;
    // reset counter to idle
    ps2_isr_func = ps2_start_bit;
}

void ps2_parity_bit()
{
    // parity bit, lets ignore it to begin with
    ps2_state.counter++;
    ps2_isr_func = ps2_stop_bit;
}

void ps2_data_bits()
{
    ps2_state.buffer |= ((PIND & (1 << PS2_DAT_PIN)) >> PS2_DAT_PIN) << ps2_state.counter;
    ps2_state.counter++;
    ps2_isr_func = (ps2_state.counter < 8) ? ps2_data_bits : ps2_parity_bit;
}

void ps2_start_bit()
{
    ps2_state.buffer = 0;
    ps2_state.counter = 0;
    ps2_isr_func = ps2_data_bits;
}

ISR(INT1_vect)
{
    (*ps2_isr_func)();
}

void setup()
{
    Serial.begin(115200);

    // initialize PS/2 state
    ps2_buffered.counter = PS2_IDLE;
    ps2_buffered.buffer = 0;

    pinMode(PS2_CLK_PIN, INPUT_PULLUP);
    pinMode(PS2_DAT_PIN, INPUT_PULLUP);

    ps2_isr_func = ps2_start_bit;

    // enable interrupt on INT0
    EICRA = (EICRA & ~((1 << ISC10) | (1 << ISC11))) | (FALLING << ISC10);
    EIMSK |= (1 << INT1);
}

void loop()
{
    char s[80];
    if (ps2_buffered.counter != PS2_IDLE)
    {
        uint8_t key = ps2_buffered.buffer;
        ps2_buffered.counter = PS2_IDLE;

        sprintf(s, "Received key: 0x%x\n", ps2_buffered.buffer);
        Serial.println(s);
    }
}