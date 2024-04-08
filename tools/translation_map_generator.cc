#include <iomanip>
#include <iostream>

#include "src/scancodes.hh"

auto init_translation_map(uint8_t translation_map[256]) -> void {
    using namespace scancodes;
    // fn line
    translation_map[ps2::ESC] = 0x45; // ESC
    translation_map[ps2::F1] = 0x50;  // F1
    translation_map[ps2::F2] = 0x51;  // F2
    translation_map[ps2::F3] = 0x52;  // F3
    translation_map[ps2::F4] = 0x53;  // F4
    translation_map[ps2::F5] = 0x54;  // F5
    translation_map[ps2::F6] = 0x55;  // F6
    translation_map[ps2::F7] = 0x56;  // F7
    translation_map[ps2::F8] = 0x57;  // F8
    translation_map[ps2::F9] = 0x58;  // F9
    translation_map[ps2::F10] = 0x59; // F10
    translation_map[ps2::F11] = 0x5F; // HELP on F11

    // zero line
    translation_map[ps2::BACKQUOTE] = 0x00; // `
    translation_map[ps2::ONE] = 0x01;       // 1
    translation_map[ps2::TWO] = 0x02;       // 2
    translation_map[ps2::THREE] = 0x03;     // 3
    translation_map[ps2::FOUR] = 0x04;      // 4
    translation_map[ps2::FIVE] = 0x05;      // 5
    translation_map[ps2::SIX] = 0x06;       // 6
    translation_map[ps2::SEVEN] = 0x07;     // 7
    translation_map[ps2::EIGHT] = 0x08;     // 8
    translation_map[ps2::NINE] = 0x09;      // 9
    translation_map[ps2::ZERO] = 0x0A;      // 0
    translation_map[ps2::HYPHEN] = 0x0B;    // 0
    translation_map[ps2::EQUAL] = 0x0C;     // -
    translation_map[ps2::BACKSPACE] = 0x41; // BACKSPACE

    // first letter line
    translation_map[ps2::TAB] = 0x42;            // TAB
    translation_map[ps2::Q] = 0x10;              // Q
    translation_map[ps2::W] = 0x11;              // W
    translation_map[ps2::E] = 0x12;              // E
    translation_map[ps2::R] = 0x13;              // R
    translation_map[ps2::T] = 0x14;              // T
    translation_map[ps2::Y] = 0x15;              // Y
    translation_map[ps2::U] = 0x16;              // U
    translation_map[ps2::I] = 0x17;              // I
    translation_map[ps2::O] = 0x18;              // O
    translation_map[ps2::P] = 0x19;              // P
    translation_map[ps2::OPENED_BRACKET] = 0x1A; // [
    translation_map[ps2::CLOSED_BRACKET] = 0x1B; // ]

    // second letter line
    translation_map[ps2::CAPS_LOCK] = 0x62;  // CAPS LOCK
    translation_map[ps2::A] = 0x20;          // A
    translation_map[ps2::S] = 0x21;          // S
    translation_map[ps2::D] = 0x22;          // D
    translation_map[ps2::F] = 0x23;          // F
    translation_map[ps2::G] = 0x24;          // G
    translation_map[ps2::H] = 0x25;          // H
    translation_map[ps2::J] = 0x26;          // J
    translation_map[ps2::K] = 0x27;          // K
    translation_map[ps2::L] = 0x28;          // L
    translation_map[ps2::SEMI_COLON] = 0x29; // ;
    translation_map[ps2::QUOTE] = 0x2A;      // '
    // translation_map[0x5D] = ?; // ?
    translation_map[ps2::ENTER] = 0x44; // ENTER

    // third letter line
    translation_map[ps2::LEFT_SHIFT] = 0x60;           // LEFT SHIFT
    translation_map[ps2::OPENED_ANGLE_BRACKET] = 0x0D; // BACKSLASH
    translation_map[ps2::Z] = 0x31;                    // X
    translation_map[ps2::X] = 0x32;                    // A
    translation_map[ps2::C] = 0x33;                    // C
    translation_map[ps2::V] = 0x34;                    // V
    translation_map[ps2::B] = 0x35;                    // B
    translation_map[ps2::N] = 0x36;                    // N
    translation_map[ps2::M] = 0x37;                    // M
    translation_map[ps2::COMMA] = 0x38;                // , (<)
    translation_map[ps2::DOT] = 0x39;                  // . (>)
    translation_map[ps2::SLASH] = 0x3A;                // /
    translation_map[ps2::RIGHT_SHIFT] = 0x61;          // RIGHT SHIFT

    // last line
    translation_map[ps2::LEFT_CTRL] = 0x63;            // CTRL
    translation_map[ps2::extended::LEFT_OS] = 0x66;    // LEFT AMIGA
    translation_map[ps2::LEFT_ALT] = 0x64;             // LEFT ALT
    translation_map[ps2::SPACE] = 0x40;                // SPACE
    translation_map[ps2::extended::RIGHT_ALT] = 0x65;  // RIGHT ALT
    translation_map[ps2::extended::RIGHT_OS] = 0x67;   // RIGHT AMIGA
    translation_map[ps2::extended::RIGHT_CTRL] = 0x63; // CTRL

    translation_map[ps2::extended::DELETE] = 0x46; // DELETE

    // cursors
    translation_map[ps2::extended::UP] = 0x4C;    // CURSOR UP
    translation_map[ps2::extended::DOWN] = 0x4D;  // CURSOR DOWN
    translation_map[ps2::extended::LEFT] = 0x4F;  // CURSOR LEFT
    translation_map[ps2::extended::RIGHT] = 0x4E; // CURSOR RIGHT

    // num pad
    translation_map[ps2::NUM_LOCK] = 0x5A;            // (
    translation_map[ps2::extended::NUM_SLASH] = 0x5B; // )
    translation_map[ps2::NUM_ASTERISK] = 0x5C;        // /
    translation_map[ps2::NUM_MINUS] = 0x5D;           // *
    translation_map[ps2::NUM_SEVEN] = 0x3D;           // 7
    translation_map[ps2::NUM_EIGHT] = 0x3E;           // 8
    translation_map[ps2::NUM_NINE] = 0x3F;            // 9
    translation_map[ps2::NUM_PLUS] = 0x5E;            // +
    translation_map[ps2::NUM_FOUR] = 0x2D;            // 4
    translation_map[ps2::NUM_FIVE] = 0x2E;            // 5
    translation_map[ps2::NUM_SIX] = 0x2F;             // 6
    translation_map[ps2::NUM_ONE] = 0x1D;             // 1
    translation_map[ps2::NUM_TWO] = 0x1E;             // 2
    translation_map[ps2::NUM_THREE] = 0x1F;           // 3
    translation_map[ps2::NUM_ZERO] = 0x0F;            // 0
    translation_map[ps2::NUM_DOT] = 0x3C;             // .
    translation_map[ps2::extended::NUM_ENTER] = 0x43; // ENTER
}

int main() {
    uint8_t translation_map[256] = {0};
    init_translation_map(translation_map);

    std::cout << "#pragma once" << std::endl << std::endl;
    std::cout << "#include <avr/pgmspace.h>" << std::endl << std::endl;
    std::cout << "const uint8_t ps2_scancode_to_amiga_keycode[256] PROGMEM = {"
              << std::endl;
    for (int i = 0; i < 256; i += 8) {
        std::cout << "    ";
        for (int j = 0; j < 8; ++j) {
            std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2)
                      << (int)translation_map[i + j] << ", ";
        }
        std::cout << std::endl;
    }
    std::cout << "};" << std::endl;

    return 0;
}