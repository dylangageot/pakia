# Plan

# 1. Implement IBM AT PS/2 protocol

> The PS/2 keyboard port is electrically and logically identical to the IBM AT keyboard port, differing only in the type of electrical connector used. - [Wikipedia](https://en.wikipedia.org/wiki/PS/2_port)

The nature of the protocol is synchronous master/slave bidirectional, with favor from the keyboard to the host.
Data and clock signals are both open-collector controlled on each side.
The keyboard generates the clock signal, even when the host is sending data to the device.
The host controls the directions by pulling the clock line to LOW.
When the clock is set to LOW from the host, the keyboard is inhibited.
The keyboard should be able to see that and will stop transmitting data.


***Pinout of female DIN 5-pin:***
```
     .---. .---.
    /    |_|    \
   /             \    1: Clock
  | 3           1 |   2: Data
  | o  5     4  o |   3: Reset(XT Type-1)
   \  o   2   o  /    4: GND
    \     o     /     5: 5V
     `--.___,--'
```

***Sources:***
- https://github.com/tmk/tmk_keyboard/wiki/IBM-PC-AT-Keyboard-Protocol
- https://en.wikipedia.org/wiki/PS/2_port

## Implementation ideas

- Use interrupt on clock line to receive data
- When the clock is being controlled by the keyboard, use the corresponding pin in input mode
- When the clock has to be pulled to the low state by the host, use the corresponding pin in output with LOW state

## Todo

- ✅ First research on IBM protocol
- ✅ Cut the wire in half, solder sub-wires to the female DIN connector
- ✅ Solder HE14 header pin on the other side
- ✅ Acquire through an interrupt scan code from the keyboard
- ⬛ Communicate with the keyboard (set Num Lock, Caps Lock...)

## Correspondance scan code set 2

```
        ,-----------------------------------------------.
        |F13|F14|F15|F16|F17|F18|F19|F20|F21|F22|F23|F24|
,---.   |-----------------------------------------------|     ,-----------.     ,-----------.
|Esc|   |F1 |F2 |F3 |F4 |F5 |F6 |F7 |F8 |F9 |F10|F11|F12|     |PrS|ScL|Pau|     |VDn|VUp|Mut|
`---'   `-----------------------------------------------'     `-----------'     `-----------'
,-----------------------------------------------------------. ,-----------. ,---------------.
|  `|  1|  2|  3|  4|  5|  6|  7|  8|  9|  0|  -|  =|JPY|Bsp| |Ins|Hom|PgU| |NmL|  /|  *|  -|
|-----------------------------------------------------------| |-----------| |---------------|
|Tab  |  Q|  W|  E|  R|  T|  Y|  U|  I|  O|  P|  [|  ]|  \  | |Del|End|PgD| |  7|  8|  9|  +|
|-----------------------------------------------------------| `-----------' |---------------|
|CapsL |  A|  S|  D|  F|  G|  H|  J|  K|  L|  ;|  '|  #|Entr|               |  4|  5|  6|KP,|
|-----------------------------------------------------------|     ,---.     |---------------|
|Shft|  <|  Z|  X|  C|  V|  B|  N|  M|  ,|  .|  /| RO|Shift |     |Up |     |  1|  2|  3|Ent|
|-----------------------------------------------------------| ,-----------. |---------------|
|Ctl|Gui|Alt|MHEN|     Space      |HENK|KANA|Alt|Gui|App|Ctl| |Lef|Dow|Rig| |  #|  0|  .|KP=|
`-----------------------------------------------------------' `-----------' `---------------'

        ,-----------------------------------------------.
        | 08| 10| 18| 20| 28| 30| 38| 40| 48| 50| 57| 5F|
,---.   |-----------------------------------------------|     ,-----------.     ,-----------.
| 76|   | 05| 06| 04| 0C| 03| 0B| 83| 0A| 01| 09| 78| 07|     |+7C| 7E|+77|     |*21|*32|*23|
`---'   `-----------------------------------------------'     `-----------'     `-----------'
,-----------------------------------------------------------. ,-----------. ,---------------.
| 0E| 16| 1E| 26| 25| 2E| 36| 3D| 3E| 46| 45| 4E| 55| 6A| 66| |*70|*6C|*7D| | 77|*4A| 7C| 7B|
|-----------------------------------------------------------| |-----------| |---------------|
| 0D  | 15| 1D| 24| 2D| 2C| 35| 3C| 43| 44| 4D| 54| 5B|  5D | |*71|*69|*7A| | 6C| 75| 7D| 79|
|-----------------------------------------------------------| `-----------' |---------------|
| 58   | 1C| 1B| 23| 2B| 34| 33| 3B| 42| 4B| 4C| 52|^5D| 5A |               | 6B| 73| 74| 6D|
|-----------------------------------------------------------|     ,---.     |---------------|
| 12 | 61| 1A| 22| 21| 2A| 32| 31| 3A| 41| 49| 4A| 51|  59  |     |*75|     | 69| 72| 7A|*5A|
|-----------------------------------------------------------| ,-----------. |---------------|
| 14|*1F| 11| 67 |     29         | 64 | 13 |*11|*27|*2F|*14| |*6B|*72|*74| | 68| 70| 71| 63|
`-----------------------------------------------------------' `-----------' `---------------'
*: E0-prefixed codes
+7C: E0 12 E0 7C
+77: E1 14 77 E1 F0 14 F0 77
^: ISO hash key uses identical scancode 5D to US backslash.
51, 63, 68, 6A, 6D: Hidden keys in IBM model M [6]
```

# 2. Implement Amiga keyboard protocol

KCLK is managed by keyboard.
KDATA is managed by both.
Active low signal levels.
Open collector setup.

Generate Amiga keybaord clock with Clear Time on Capture Match, both generating the oscillating clock and the ability to tweak data line through an interrupt.

https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf page 79

- ✅ Use timer and interrupt to implement CLOCK and DATA positioning for Amiga protocol
- ⬛ Handle ACK from device on keyboard side (Arduino)
- ⬛ Translate scancode to Amiga scancodes
