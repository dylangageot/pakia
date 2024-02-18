#define __AVR_ATmega328P__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <Arduino.h>

#include "ps2.hh"

volatile struct ps2_frame ps2_frames[PS2_FRAME_COUNT_POW_2];
volatile struct ps2_fsm ps2_fsm;

void ps2_process_start_bit();
void ps2_process_data_bits();
void ps2_process_parity_bit();
void ps2_process_stop_bit();

void ps2_fsm::begin()
{
    frame_buffer_index = 0;
    state = ps2_process_start_bit;

    // initialize PS/2 pin state
    pinMode(PS2_CLK_PIN, INPUT_PULLUP);
    pinMode(PS2_DAT_PIN, INPUT_PULLUP);

    // enable interrupt on INT0
    EICRA = (EICRA & ~((1 << ISC10) | (1 << ISC11))) | (FALLING << ISC10);
    EIMSK |= (1 << INT1);
}

ISR(INT1_vect)
{
    (*ps2_fsm.state)();
}

static void ps2_process_start_bit()
{
    ps2_fsm.buffer = 0;
    ps2_fsm.counter = 0;
    ps2_fsm.state = ps2_process_data_bits;
}

static void ps2_process_data_bits()
{
    ps2_fsm.buffer |= ((PIND & (1 << PS2_DAT_PIN)) >> PS2_DAT_PIN) << ps2_fsm.counter++;
    ps2_fsm.state = (ps2_fsm.counter < 8) ? ps2_process_data_bits : ps2_process_parity_bit;
}

static void ps2_process_parity_bit()
{
    // parity bit, lets ignore it to begin with
    ps2_fsm.counter++;
    ps2_fsm.state = ps2_process_stop_bit;
}

static void ps2_process_stop_bit()
{
    // save data
    volatile struct ps2_frame *frame = (ps2_frames + (ps2_fsm.frame_buffer_index++ & (PS2_FRAME_COUNT_POW_2 - 1)));
    frame->available = true;
    frame->key = ps2_fsm.buffer;
    // reset counter to idle
    ps2_fsm.state = ps2_process_start_bit;
}
