#include <iostream>
#include <iomanip>

void init_translation_map(uint8_t translation_map[256])
{
    // fn line
    translation_map[0x76] = 0x45; // ESC
    translation_map[0x05] = 0x50; // F1
    translation_map[0x06] = 0x51; // F2
    translation_map[0x04] = 0x52; // F3
    translation_map[0x0C] = 0x53; // F4
    translation_map[0x03] = 0x54; // F5
    translation_map[0x0B] = 0x55; // F6
    translation_map[0x83] = 0x56; // F7
    translation_map[0x0A] = 0x57; // F8
    translation_map[0x01] = 0x58; // F9
    translation_map[0x09] = 0x59; // F10
    translation_map[0x78] = 0x5F; // HELP on F11

    // zero line
    translation_map[0x0E] = 0x00; // `
    translation_map[0x16] = 0x01; // 1
    translation_map[0x1E] = 0x02; // 2
    translation_map[0x26] = 0x03; // 3
    translation_map[0x25] = 0x04; // 4
    translation_map[0x2E] = 0x05; // 5
    translation_map[0x36] = 0x06; // 6
    translation_map[0x3D] = 0x07; // 7
    translation_map[0x3E] = 0x08; // 8
    translation_map[0x46] = 0x09; // 9
    translation_map[0x45] = 0x0A; // 0
    translation_map[0x4e] = 0x0B; // 0
    translation_map[0x55] = 0x0C; // -
    translation_map[0x66] = 0x41; // BACKSPACE

    // first letter line
    translation_map[0x0D] = 0x42; // TAB
    translation_map[0x15] = 0x10; // Q
    translation_map[0x1D] = 0x11; // W
    translation_map[0x24] = 0x12; // E
    translation_map[0x2D] = 0x13; // R
    translation_map[0x2C] = 0x14; // T
    translation_map[0x35] = 0x15; // Y
    translation_map[0x3C] = 0x16; // U
    translation_map[0x43] = 0x17; // I
    translation_map[0x44] = 0x18; // O
    translation_map[0x4D] = 0x19; // P
    translation_map[0x54] = 0x1A; // [
    translation_map[0x5B] = 0x1B; // ]

    // second letter line
    translation_map[0x58] = 0x62; // CAPS LOCK
    translation_map[0x1C] = 0x20; // A
    translation_map[0x1B] = 0x21; // S
    translation_map[0x23] = 0x22; // D
    translation_map[0x2B] = 0x23; // F
    translation_map[0x34] = 0x24; // G
    translation_map[0x33] = 0x25; // H
    translation_map[0x3B] = 0x26; // J
    translation_map[0x42] = 0x27; // K
    translation_map[0x4B] = 0x28; // L
    translation_map[0x4C] = 0x29; // ;
    translation_map[0x52] = 0x2A; // '
    // translation_map[0x5D] = ?; // ?
    translation_map[0x5A] = 0x44; // ENTER

    // third letter line
    translation_map[0x12] = 0x60; // LEFT SHIFT
    translation_map[0x61] = 0x0D; // BACKSLASH
    translation_map[0x1A] = 0x31; // X
    translation_map[0x22] = 0x32; // A
    translation_map[0x21] = 0x33; // C
    translation_map[0x2A] = 0x34; // V
    translation_map[0x32] = 0x35; // B
    translation_map[0x31] = 0x36; // N
    translation_map[0x3A] = 0x37; // M
    translation_map[0x41] = 0x38; // , (<)
    translation_map[0x49] = 0x39; // . (>)
    translation_map[0x4A] = 0x3A; // /
    translation_map[0x59] = 0x61; // RIGHT SHIFT

    // last line
    translation_map[0x14] = 0x63;            // CTRL
    translation_map[(1 << 7) | 0x1F] = 0x66; // LEFT AMIGA
    translation_map[0x11] = 0x64;            // LEFT ALT
    translation_map[0x29] = 0x40;            // SPACE
    translation_map[(1 << 7) | 0x11] = 0x65; // RIGHT ALT
    translation_map[(1 << 7) | 0x27] = 0x67; // RIGHT AMIGA
    translation_map[(1 << 7) | 0x14] = 0x63; // CTRL

    translation_map[(1 << 7) | 0x71] = 0x46; // DELETE

    // cursors
    translation_map[(1 << 7) | 0x75] = 0x4C; // CURSOR UP
    translation_map[(1 << 7) | 0x72] = 0x4D; // CURSOR DOWN
    translation_map[(1 << 7) | 0x6B] = 0x4F; // CURSOR LEFT
    translation_map[(1 << 7) | 0x74] = 0x4E; // CURSOR RIGHT

    // num pad
    translation_map[0x77] = 0x5A;            // (
    translation_map[(1 << 7) | 0x4A] = 0x5C; // /
    translation_map[0x7C] = 0x5D;            // *
    translation_map[0x7B] = 0x4A;            // -
    translation_map[0x6C] = 0x3D;            // 7
    translation_map[0x75] = 0x3E;            // 8
    translation_map[0x7D] = 0x3F;            // 9
    translation_map[0x79] = 0x5E;            // +
    translation_map[0x6B] = 0x2D;            // 4
    translation_map[0x73] = 0x2E;            // 5
    translation_map[0x74] = 0x2F;            // 6
    translation_map[0x69] = 0x1D;            // 1
    translation_map[0x72] = 0x1E;            // 2
    translation_map[0x7A] = 0x1F;            // 3
    translation_map[0x70] = 0x0F;            // 0
    translation_map[0x71] = 0x3C;            // .
    translation_map[(1 << 7) | 0x5A] = 0x43; // ENTER
}

int main()
{
    uint8_t translation_map[256] = {0};
    init_translation_map(translation_map);

    std::cout << "#pragma once" << std::endl << std::endl;
    std::cout << "const uint8_t ps2_scancode_to_amiga_keycode[256] PROGMEM = {" << std::endl;
    for (int i = 0; i < 256; i += 8)
    {
        std::cout << "    ";
        for (int j = 0; j < 8; ++j)
        {
            std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2) << (int)translation_map[i + j] << ", ";
        }
        std::cout << std::endl;
    }
    std::cout << "};" << std::endl;

    return 0;
}