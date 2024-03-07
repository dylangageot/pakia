#define __AVR_ATmega328P__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <Arduino.h>

#include "ps2.hh"

namespace ps2
{

    struct circular_buffer<frame> frames;
    struct fsm fsm;

    inline static void set_pins_pull_up()
    {
        DDRD &= ~((1 << CLK_PIN) | (1 << DAT_PIN));
        PORTD |= (1 << CLK_PIN) | (1 << DAT_PIN);
    }

    inline static void set_clk_low()
    {
        PORTD &= ~(1 << CLK_PIN);
        DDRD &= ~(1 << CLK_PIN);
    }

    inline static void setup_clk_detection_int()
    {
        EICRA = (EICRA & ~((1 << ISC10) | (1 << ISC11))) | (FALLING << ISC10);
    }

    inline static void enable_clk_detection_int()
    {
        EIMSK |= (1 << INT1);
    }

    inline void disable_clk_detection_int()
    {
        EIMSK &= ~(1 << INT1);
    }

    inline void setup_timer()
    {
        TCCR0A = 0;
        TCCR0B = (1 << CS02) | (1 << CS00);
        TCNT0 = 100;
    }

    inline void enable_timer_int()
    {
        TIFR0 &= ~(1 << TOV0);
        TIMSK0 |= (1 << TOIE0);
    }

    inline void disable_timer_int()
    {
        TIMSK0 &= ~(1 << TOIE0);
    }

    inline void inhibit()
    {
        // lets restore the communications in 20ms
        disable_clk_detection_int();
        set_clk_low();
        setup_timer();
        enable_timer_int();
    }

    static void process_start_bit();
    static void process_stop_bit()
    {
        if (!(PIND & (1 << DAT_PIN)))
        {
            inhibit();
            return;
        }
        volatile frame *frame = frames.write_iterator().get();
        // save data
        frame->scancode = fsm.buffer;
        frame->available = true;
        // move to next frame pointer, make it unavailable
        frames.write_iterator().next();
        // reset counter to idle
        fsm.state = process_start_bit;
    }

    static void process_parity_bit()
    {
        uint8_t read_parity = (PIND & (1 << DAT_PIN)) >> DAT_PIN;
        uint8_t computed_parity = fsm.buffer;
        computed_parity ^= computed_parity >> 4;
        computed_parity ^= computed_parity >> 2;
        computed_parity ^= computed_parity >> 1;
        computed_parity = (~computed_parity) & 1;
        if (read_parity != computed_parity)
        {
            inhibit();
            return;
        }
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
        if (PIND & (1 << DAT_PIN))
        {
            inhibit();
            return;
        }
        fsm.buffer = 0;
        fsm.counter = 0;
        fsm.state = process_data_bits;
    }

    void begin()
    {
        fsm.begin();
    }

    void fsm::begin()
    {
        state = process_start_bit;
        set_pins_pull_up();
        setup_clk_detection_int();
        enable_clk_detection_int();
    }

    ISR(INT1_vect)
    {
        (*fsm.state)();
    }

    ISR(TIMER0_OVERFLOW_vect)
    {
        disable_timer_int();
        fsm.state = process_start_bit;
        set_pins_pull_up();
        enable_clk_detection_int();
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
                return event.scancode != 0;
            }
        }
        return false;
    }

}