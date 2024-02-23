#define __AVR_ATmega328P__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <Arduino.h>

#include "ps2.hh"

namespace ps2
{

    volatile frame frames[FRAME_COUNT];
    struct fsm fsm;

    void ps2_process_start_bit();
    void ps2_process_data_bits();
    void ps2_process_parity_bit();
    void ps2_process_stop_bit();

    void fsm::begin()
    {
        state = ps2_process_start_bit;

        // initialize PS/2 pin state
        pinMode(CLK_PIN, INPUT_PULLUP);
        pinMode(DAT_PIN, INPUT_PULLUP);

        // enable interrupt on INT0
        EICRA = (EICRA & ~((1 << ISC10) | (1 << ISC11))) | (FALLING << ISC10);
        EIMSK |= (1 << INT1);
    }

    ISR(INT1_vect)
    {
        (*fsm.state)();
    }

    static void ps2_process_start_bit()
    {
        fsm.buffer = 0;
        fsm.counter = 0;
        fsm.state = ps2_process_data_bits;
    }

    static void ps2_process_data_bits()
    {
        fsm.buffer |= ((PIND & (1 << DAT_PIN)) >> DAT_PIN) << fsm.counter++;
        fsm.state = (fsm.counter < 8) ? ps2_process_data_bits : ps2_process_parity_bit;
    }

    static void ps2_process_parity_bit()
    {
        // parity bit, lets ignore it to begin with
        fsm.counter++;
        fsm.state = ps2_process_stop_bit;
    }

    static void ps2_process_stop_bit()
    {
        volatile frame *frame = fsm.iterator.get();
        // save data
        frame->scancode = fsm.buffer;
        frame->available = true;
        // move forward frame pointer and wrap around if needed
        fsm.iterator.move_forward();
        // reset counter to idle
        fsm.state = ps2_process_start_bit;
    }

    parser::parser()
    {
        reset();
    }

    bool parser::consume(frame_iterator *iterator, event *event)
    {
        volatile frame *frame = iterator->get();
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
                event->event_kind = _is_released ? RELEASED : PRESSED;
                event->scancode = _last_scancode_fed | (_is_e0_prefixed ? 1 << 7 : 0);
                reset();
                return true;
            }
        }
        return false;
    }

    void parser::reset()
    {
        _is_e0_prefixed = false;
        _is_released = false;
        _last_scancode_fed = 0;
    }

}