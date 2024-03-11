#include <avr/io.h>
#include <avr/interrupt.h>

#include "pins.hh"

ISR(TIM1_OVF_vect)
{
    //PORTB ^= pins::amiga::DAT;
}

ISR(PCINT0_vect)
{
    
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

    // ps2 inihibt
    // TCCR0A = (1 << WGM01); // CTC
    // TCCR0B = (1 << CS02) | (1 << CS00); // f_clk_io, no prescaling
    // TCNT0 = 0;
    // OCR0A = 155;
    // TIFR |= (1 << OCF0A);
    // TIMSK |= (1 << OCIE0A);

    // // amiga bitbanging timer setup
    // 20u config
    // TCCR1 = (1 << PWM1A) | (1 << CS10); // f_clk_io, no prescaling
    // GTCCR &= (1 << TSM) | (1 << PSR0);
    // TCNT1 = 0;
    // OCR1C = 159;
    // TIFR |= (1 << TOV1);
    // TIMSK |= (1 << TOIE1);

    // 143ms config
    TCCR1 = (1 << PWM1A) | (1 << CS13) | (1 << CS12) | (1 << CS11); // f_clk_io, no prescaling
    GTCCR &= (1 << TSM) | (1 << PSR0);
    TCNT1 = 0;
    OCR1C = 138;
    TIFR |= (1 << TOV1);
    TIMSK |= (1 << TOIE1);

    // ack interrupt
    GIFR |= (1 << PCIF);
    GIMSK |= (1 << PCIE);
    PCMSK |= (1 << PCINT4);

    while (1)
    {
    }
}