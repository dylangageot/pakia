#pragma once

// ATTiny85 pinout:
//             +---\/---+
//        PB5  |1*     8|  VCC
//        PB3  |2      7|  PB2 (INT0)
//        PB4  |3      6|  PB1
//        GND  |4      5|  PB0
//             +--------+

namespace pins
{

    namespace amiga
    {
        const uint8_t RST = 0;
        const uint8_t CLK = 1 << 3;
        const uint8_t DAT = 1 << 4;
    }

    namespace ps2
    {
        const uint8_t DAT_PIN = 1;
        const uint8_t DAT = 1 << DAT_PIN;
        const uint8_t CLK = 1 << 2;
    }

} // namespace pins
