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
- ⬛ Label the wire according to the pinout above
- ⬛ Solder HE14 header pin on the other side
