#define __AVR_ATmega328P__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <Arduino.h>

#include "ps2.hh"

volatile ps2_frame_t ps2_frames[PS2_FRAME_COUNT];
volatile ps2_fsm_t ps2_fsm;

void ps2_process_start_bit();
void ps2_process_data_bits();
void ps2_process_parity_bit();
void ps2_process_stop_bit();

void ps2_fsm_t::begin()
{
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
    volatile ps2_frame_t *frame = ps2_fsm.iterator.get();
    // save data
    frame->scancode = ps2_fsm.buffer;
    frame->available = true;
    // move forward frame pointer and wrap around if needed
    ps2_fsm.iterator.move_forward();
    // reset counter to idle
    ps2_fsm.state = ps2_process_start_bit;
}

ps2_parser_t::ps2_parser_t()
{
    reset();
}

bool ps2_parser_t::consume(ps2_frame_iterator_t *iterator)
{
    volatile ps2_frame_t *frame = iterator->get();
    if (frame->available)
    {
        _last_scancode_fed = frame->scancode;
        frame->available = false;
        iterator->move_forward();
        if (_last_scancode_fed == 0xF0)
        {
            _is_released = true;
        }
        else if (_last_scancode_fed == 0xE0)
        {
            _is_e0_prefixed = true;
        }
        else
        {
            return true;
        }
    }
    return false;
}

ps2_event_t ps2_parser_t::get_event()
{
    ps2_event_t event;
    event.event_kind = _is_released ? RELEASED : PRESSED;
    event.scancode = _last_scancode_fed | (_is_e0_prefixed ? 1 << 7 : 0);
    reset();
    return event;
}

void ps2_parser_t::reset()
{
    _is_e0_prefixed = false;
    _is_released = false;
    _last_scancode_fed = 0;
}