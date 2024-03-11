#include <avr/io.h>
#include <avr/interrupt.h>

#include "pins.hh"

ISR(INT0_vect)
{
    PORTB ^= pins::amiga::DAT;
}

int main()
{

    // enable pull-up
    MCUCR &= ~(1 << PUD);

    // ps2::CLK and ps2::DAT as input (with pull-up)
    DDRB &= ~(pins::ps2::CLK | pins::ps2::DAT);
    PORTB |= pins::ps2::CLK | pins::ps2::DAT;

    // ps2::CLK interrupt enable
    MCUCR = (MCUCR & ~((1 << ISC01) | (1 << ISC00))) | (1 << ISC01);
    GIFR |= (1 << INTF0);
    GIMSK |= (1 << INT0);
    SREG |= (1 << SREG_I);

    // amiga::CLK and amiga::DAT as output
    DDRB |= pins::amiga::CLK | pins::amiga::DAT;
    PORTB |= pins::amiga::CLK | pins::amiga::DAT;

    while (1)
    {
    }
}