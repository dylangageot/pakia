# PS/2 to Amiga Keyboard Interface Adapter (PAKIA)

Standalone Amiga keyboards are more than ever difficult to find, especially due to the increased perceived value of Amiga computers and peripherals on the second-hand market.

Unfortunately, the keyboard interface used by Commodore is proprietary, see the [Amiga keyboard interface documentation](http://amigadev.elowar.com/read/ADCD_2.1/Hardware_Manual_guide/node0172.html).
Even though it uses the same 5-pin DIN 41524 connector as IBM AT keyboards, the keyboard interface is still different and therefore not compatible with any easy-to-find PC-compatible keyboards.

PAKIA is yet another adapter circuit, taking a PS/2 keyboard interface in input, and translating it into the Amiga keyboard interface in output.

## Features

- Convert PS/2 scancode to Amiga scancode,
- Handle lost synchronization and corresponding recovery,
- Use the [Menu key](https://en.wikipedia.org/wiki/Menu_key) modifier to type a key that has been absorbed by a larger key on the PS/2 keyboard but is present on an Amiga keyboard.

## Under the hood

Despite being different, both PS/2 and Amiga keyboard interfaces share similarities: they use the same KDAT and KCLK signal pair.
In either case, the keyboard is responsible for generating the clock and preparing 8 data bits to be read on the KDAT signal a falling or rising edge of the KCLK signal.

Leveraging multiple timers and interrupt service routines, an ATtiny85 captures frames sent out from an input PS/2 keyboard, interprets them, and then converts them to the Amiga keyboard interface using a look-up table written in Flash memory.

In contrast to other solutions, signal timings for the Amiga keyboard interface are driven by discrete time events instead of being driven by blocking delays.
One hardware timer generates periodic interrupt requests that once the corresponding ISR runs, the signal is taught to evolve its state through a software-based finite state machine, hence changing pins levels for both KDAT and KCLK.

This asynchronous philosophy makes it possible to incorporate this piece of software in a busier system, without introducing blocking delay when Amiga commands are being sent (minus interrupting service routines execution time of course).

You might think that this is overengineering and that other solutions got it right enough by using blocking delays to handle their signal timings, and you are probably right!
The solution I have implemented was way more stimulating for me to develop, so here we are!

The binary currently weighs 2KB, in that regard, one could choose to use an ATtiny45 instead of an ATtiny85, it might be enough even in case of additional features coming in future updates.

## Build and deploy

> **Warning:** The build system is only available on Linux.

> **Required:** `avr-gcc` and `avrdude`, respectively 5.4.0 and 6.3-20171130 on my setup.

The project uses C++ mainly to benefit from namespaces.

1. Specify the programmer to use with `avrdude` in the Makefile, with the variable `PROGRAMMER`.
2. Build with `make`.
3. Deploy with `make flash` while your programmer is wired to the ISP connector on the PCB.

The lookup table to convert PS/2 scancode to Amiga ones is hard-coded in `src/translation_map.hh`.
A tool called `translation-map-generator` can be built from the `tools` directory.
By modifying the source code, one can generate other hard-coded LUT from a readable code.

##  Schematic and PCB

The schematic and PCB were drawn in KiCAD. The project is available in the `hardware` directory.

## Other solutions

> "yet another adapter circuit", is there are other adapter circuits?

Yes, there are other adapter circuits! 

I started this project blindly, without searching if anyone made one before.
Frankly, it was appealing to my eye, so why not?
I found documentation on both keyboard interfaces and started developing a proof of concept on an Arduino Uno board, based on an ATmega328p.

I discovered many of them once I had finished the prototype for PAKIA and wanted to design a PCB for it, with eventually a lighter microcontroller than the ATmega328p.
Here is a list of related open-source projects and their respective solutions:

- [AKAB](https://gitlab.com/hkzlab-retrocomputing/AKAB_Reloaded) by [hkzlabnet](http://mercatopo-en.blogspot.com/)
    - uses an ATmega328p as a microcontroller (32KB Flash, 2KB SRAM),
    - leverages a pure AVR development environment,
    - provides a KiCAD project with schematic and PCB design,
    - PCB based on both through-hole DIP-28 ATmega328p and DIN socket,
    - see a photo of the adapter board in [the dedicated blog article](http://mercatopo-en.blogspot.com/2013/11/first-prototype-of-akab-amiga-keyboard.html),
    - implements Amiga Ctrl-Amiga-Amiga reset combo keys, 
- [PS2toAmiga](https://github.com/Jartza/PS2toAmiga) by [Jartza](https://github.com/Jartza)
    - uses an ATtiny85 as a microcontroller (8K Flash, 512B SRAM),
    - leverages the Arduino development environment,
    - provides a KiCAD project with schematic and PCB design,
    - PCB based on SMD SOIC8 ATtiny85 and pads to solder wires on for both PS/2 and Amiga keyboard wires,
    - designed to fit inside a keyboard,
    - see photos of the adapter board on the repository README.md,
    - implements Amiga Ctrl-Amiga-Amiga reset combo keys, 
    - two different keyboard layouts selectable from the *Scroll Lock* key.


There is also the [Lyra 3](http://wiki.icomp.de/wiki/Lyra_3) adapter sold by Individual Computers.
This one is very complete feature-wise, but I did not want to buy one as I thought that it would spoil my pleasure of developing my own, I'm sure you get it (right?).
