#define __AVR_ATmega328P__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <Arduino.h>

#include "ps2.hh"

namespace ps2
{

    struct circular_buffer<frame> frames;
    struct fsm fsm;

    static void process_start_bit();
    static void process_stop_bit()
    {
        volatile frame *frame = frames.write_iterator().get();
        // save data
        frame->scancode = fsm.buffer;
        frame->available = true;
        // move forward frame pointer and wrap around if needed
        frames.write_iterator().next();
        // reset counter to idle
        fsm.state = process_start_bit;
    }

    static void process_parity_bit()
    {
        // parity bit, lets ignore it to begin with
        fsm.counter++;
        fsm.state = process_stop_bit;
    }

    static void process_data_bits()
    {
        fsm.buffer |= ((PIND & (1 << DAT_PIN)) >> DAT_PIN) << fsm.counter++;
        fsm.state = (fsm.counter < 8) ? process_data_bits : process_parity_bit;
    }

    static void process_start_bit()
    {
        fsm.buffer = 0;
        fsm.counter = 0;
        fsm.state = process_data_bits;
    }

    inline static void set_pins_pull_up() {
        pinMode(CLK_PIN, INPUT_PULLUP);
        pinMode(DAT_PIN, INPUT_PULLUP);
    }

    inline static void enable_external_pin_interrupt() {
        EICRA = (EICRA & ~((1 << ISC10) | (1 << ISC11))) | (FALLING << ISC10);
        EIMSK |= (1 << INT1);
    }

    void begin() {
        fsm.begin();
    }

    void fsm::begin()
    {
        state = process_start_bit;
        set_pins_pull_up();
        enable_external_pin_interrupt();
    }

    ISR(INT1_vect)
    {
        (*fsm.state)();
    }

    receiver::receiver()
    {
        reset();
    }

    void receiver::reset()
    {
        _is_e0_prefixed = false;
        _is_released = false;
        _last_scancode_fed = 0;
    }

    bool receiver::consume(event &event)
    {
        volatile frame *frame = frames.read_iterator().get();
        if (frame->available)
        {
            _last_scancode_fed = frame->scancode;
            frame->available = false;
            frames.read_iterator().next();
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
                event.event_kind = _is_released ? RELEASED : PRESSED;
                event.scancode = _last_scancode_fed | (_is_e0_prefixed ? 1 << 7 : 0);
                reset();
                return true;
            }
        }
        return false;
    }


}