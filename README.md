# PS/2 to Amiga Keyboard Interface Adapter (PAKIA)

Standalone Amiga keyboards are more than ever difficult to find, especially due to the increased perceived value of Amiga computers and peripherals on the second-hand market.

Unfortunately, the keyboard interface used by Commodore is proprietary, see the [Amiga keyboard interface documentation](http://amigadev.elowar.com/read/ADCD_2.1/Hardware_Manual_guide/node0172.html).
Even though it uses the same 5-pin DIN 41524 connector as IBM AT keyboards, the keyboard interface is still different and therefore not compatible with any easy-to-find PC-compatible keyboards.

PAKIA is yet another adapter circuit, taking a PS/2 keyboard interface in input, and translating it into the Amiga keyboard interface in output.

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
    - PCB based on SMD ATtiny85 and pads to solder wires on for both PS/2 and Amiga keyboard wires,
    - designed to fit inside a keyboard,
    - see photos of the adapter board on the repository README.md,
    - implements Amiga Ctrl-Amiga-Amiga reset combo keys, 
    - two different keyboard layouts selectable from the *Scroll Lock* key.


There is also the [Lyra 3](http://wiki.icomp.de/wiki/Lyra_3) adapter sold by Individual Computers.
This one is very complete feature-wise, but I did not want to buy one as I thought that it would spoil my pleasure of developing my own, I'm sure you get it (right?).

# Under the hood

Leveraging multiple timers and interrupt service routines, the ATTiny85 captures frames sent out from an input PS/2 keyboard, interprets them, and then converts them into a protocol understood by an Amiga machine, the Amiga keyboard interface.


