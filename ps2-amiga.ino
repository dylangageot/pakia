#include <avr/io.h>

const size_t PS2_CLK_PIN = 3;
const size_t PS2_DAT_PIN = 4;

const uint8_t PS2_IDLE = 255;

struct ps2_buffer
{
    uint8_t buffer;
    uint8_t counter;
};

volatile struct ps2_buffer ps2_state;
volatile struct ps2_buffer ps2_buffered;

void setup()
{
    Serial.begin(9600);

    // initialize PS/2 state
    ps2_state.counter = PS2_IDLE;
    ps2_state.buffer = 0;
    ps2_buffered.counter = PS2_IDLE;
    ps2_buffered.buffer = 0;

    pinMode(PS2_CLK_PIN, INPUT_PULLUP);
    pinMode(PS2_DAT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PS2_CLK_PIN), ps2_clk_falling, FALLING);
}

void ps2_clk_falling()
{
    if (ps2_state.counter < 8)
    {
        // data bits
        ps2_state.buffer |= ((PIND & (1 << PS2_DAT_PIN)) >> PS2_DAT_PIN) << ps2_state.counter;
        ps2_state.counter++;
    }
    else if (ps2_state.counter == 8)
    {
        // parity bit, lets ignore it to begin with
        ps2_state.counter++;
    }
    else if (ps2_state.counter == 9)
    {
        // stop bit
        // save data
        ps2_buffered.counter = ps2_state.counter;
        ps2_buffered.buffer = ps2_state.buffer;
        // reset counter to idle
        ps2_state.counter = PS2_IDLE;
    }
    else if (ps2_state.counter == PS2_IDLE)
    {
        // start bit
        ps2_state.buffer = 0;
        ps2_state.counter++;
    }
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