#include <iomanip>
#include <iostream>

#include "src/scancodes.hh"

auto init_translation_map(uint8_t translation_map[256]) -> void {
    using namespace scancodes;
    translation_map[ps2::ESC] = amiga::ESC;
    translation_map[ps2::F1] = amiga::F1;
    translation_map[ps2::F2] = amiga::F2;
    translation_map[ps2::F3] = amiga::F3;
    translation_map[ps2::F4] = amiga::F4;
    translation_map[ps2::F5] = amiga::F5;
    translation_map[ps2::F6] = amiga::F6;
    translation_map[ps2::F7] = amiga::F7;
    translation_map[ps2::F8] = amiga::F8;
    translation_map[ps2::F9] = amiga::F9;
    translation_map[ps2::F10] = amiga::F10;
    translation_map[ps2::F11] = amiga::HELP;

    translation_map[ps2::BACKQUOTE] = amiga::BACKQUOTE;
    translation_map[ps2::ONE] = amiga::ONE;
    translation_map[ps2::TWO] = amiga::TWO;
    translation_map[ps2::THREE] = amiga::THREE;
    translation_map[ps2::FOUR] = amiga::FOUR;
    translation_map[ps2::FIVE] = amiga::FIVE;
    translation_map[ps2::SIX] = amiga::SIX;
    translation_map[ps2::SEVEN] = amiga::SEVEN;
    translation_map[ps2::EIGHT] = amiga::EIGHT;
    translation_map[ps2::NINE] = amiga::NINE;
    translation_map[ps2::ZERO] = amiga::ZERO;
    translation_map[ps2::HYPHEN] = amiga::HYPHEN;
    translation_map[ps2::EQUAL] = amiga::EQUAL;
    translation_map[ps2::BACKSPACE] = amiga::BACKSPACE;

    translation_map[ps2::TAB] = amiga::TAB;
    translation_map[ps2::Q] = amiga::Q;
    translation_map[ps2::W] = amiga::W;
    translation_map[ps2::E] = amiga::E;
    translation_map[ps2::R] = amiga::R;
    translation_map[ps2::T] = amiga::T;
    translation_map[ps2::Y] = amiga::Y;
    translation_map[ps2::U] = amiga::U;
    translation_map[ps2::I] = amiga::I;
    translation_map[ps2::O] = amiga::O;
    translation_map[ps2::P] = amiga::P;
    translation_map[ps2::OPENED_BRACKET] = amiga::OPENED_BRACKET;
    translation_map[ps2::CLOSED_BRACKET] = amiga::CLOSED_BRACKET;

    translation_map[ps2::CAPS_LOCK] = amiga::CAPS_LOCK;
    translation_map[ps2::A] = amiga::A;
    translation_map[ps2::S] = amiga::S;
    translation_map[ps2::D] = amiga::D;
    translation_map[ps2::F] = amiga::F;
    translation_map[ps2::G] = amiga::G;
    translation_map[ps2::H] = amiga::H;
    translation_map[ps2::J] = amiga::J;
    translation_map[ps2::K] = amiga::K;
    translation_map[ps2::L] = amiga::L;
    translation_map[ps2::SEMI_COLON] = amiga::SEMI_COLON;
    translation_map[ps2::QUOTE] = amiga::QUOTE;
    translation_map[ps2::ENTER] = amiga::ENTER;

    translation_map[ps2::LEFT_SHIFT] = amiga::LEFT_SHIFT;
    translation_map[ps2::OPENED_ANGLE_BRACKET] = amiga::BACKSLASH;
    translation_map[ps2::Z] = amiga::Z;
    translation_map[ps2::X] = amiga::X;
    translation_map[ps2::C] = amiga::C;
    translation_map[ps2::V] = amiga::V;
    translation_map[ps2::B] = amiga::B;
    translation_map[ps2::N] = amiga::N;
    translation_map[ps2::M] = amiga::M;
    translation_map[ps2::COMMA] = amiga::COMMA;
    translation_map[ps2::DOT] = amiga::DOT;
    translation_map[ps2::SLASH] = amiga::SLASH;
    translation_map[ps2::RIGHT_SHIFT] = amiga::RIGHT_SHIFT;

    translation_map[ps2::LEFT_CTRL] = amiga::LEFT_CTRL;
    translation_map[ps2::extended::LEFT_OS] = amiga::LEFT_AMIGA;
    translation_map[ps2::LEFT_ALT] = amiga::LEFT_ALT;
    translation_map[ps2::SPACE] = amiga::SPACE;
    translation_map[ps2::extended::RIGHT_ALT] = amiga::RIGHT_ALT;
    translation_map[ps2::extended::RIGHT_OS] = amiga::RIGHT_AMIGA;
    translation_map[ps2::extended::RIGHT_CTRL] = amiga::RIGHT_CTRL;

    translation_map[ps2::extended::DELETE] = amiga::DELETE;

    translation_map[ps2::extended::UP] = amiga::UP;
    translation_map[ps2::extended::DOWN] = amiga::DOWN;
    translation_map[ps2::extended::LEFT] = amiga::LEFT;
    translation_map[ps2::extended::RIGHT] = amiga::RIGHT;
    
    translation_map[ps2::NUM_LOCK] = amiga::NUM_OPENED_PARENTHESIS;
    translation_map[ps2::extended::NUM_SLASH] = amiga::NUM_CLOSED_PARENTHESIS;
    translation_map[ps2::NUM_ASTERISK] = amiga::NUM_SLASH;
    translation_map[ps2::NUM_MINUS] = amiga::NUM_ASTERISK;
    translation_map[ps2::NUM_SEVEN] = amiga::NUM_SEVEN;
    translation_map[ps2::NUM_EIGHT] = amiga::NUM_EIGHT;
    translation_map[ps2::NUM_NINE] = amiga::NUM_NINE;
    translation_map[ps2::NUM_PLUS] = amiga::NUM_PLUS;
    translation_map[ps2::NUM_FOUR] = amiga::NUM_FOUR;
    translation_map[ps2::NUM_FIVE] = amiga::NUM_FIVE;
    translation_map[ps2::NUM_SIX] = amiga::NUM_SIX;
    translation_map[ps2::NUM_ONE] = amiga::NUM_ONE;
    translation_map[ps2::NUM_TWO] = amiga::NUM_TWO;
    translation_map[ps2::NUM_THREE] = amiga::NUM_THREE;
    translation_map[ps2::NUM_ZERO] = amiga::NUM_ZERO;
    translation_map[ps2::NUM_DOT] = amiga::NUM_DOT;
    translation_map[ps2::extended::NUM_ENTER] = amiga::NUM_ENTER;
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