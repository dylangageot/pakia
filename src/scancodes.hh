#pragma once

#include <stdint.h>

namespace scancodes {

    namespace ps2 {

        const uint8_t ESC = 0x76;
        const uint8_t F1 = 0x05;
        const uint8_t F2 = 0x06;
        const uint8_t F3 = 0x04;
        const uint8_t F4 = 0x0C;
        const uint8_t F5 = 0x03;
        const uint8_t F6 = 0x0B;
        const uint8_t F7 = 0x83;
        const uint8_t F8 = 0x0A;
        const uint8_t F9 = 0x01;
        const uint8_t F10 = 0x09;
        const uint8_t F11 = 0x78;
        const uint8_t F12 = 0x07;
        // todo: add F13-F24 if needed

        const uint8_t BACKQUOTE = 0x0E;
        const uint8_t ONE = 0x16;
        const uint8_t TWO = 0x1E;
        const uint8_t THREE = 0x26;
        const uint8_t FOUR = 0x25;
        const uint8_t FIVE = 0x2E;
        const uint8_t SIX = 0x36;
        const uint8_t SEVEN = 0x3D;
        const uint8_t EIGHT = 0x3E;
        const uint8_t NINE = 0x46;
        const uint8_t ZERO = 0x45;
        const uint8_t HYPHEN = 0x4E;
        const uint8_t EQUAL = 0x55;
        const uint8_t JPY = 0x6A;
        const uint8_t BACKSPACE = 0x66;

        const uint8_t TAB = 0x0D;
        const uint8_t Q = 0x15;
        const uint8_t W = 0x1D;
        const uint8_t E = 0x24;
        const uint8_t R = 0x2D;
        const uint8_t T = 0x2C;
        const uint8_t Y = 0x35;
        const uint8_t U = 0x3C;
        const uint8_t I = 0x43;
        const uint8_t O = 0x44;
        const uint8_t P = 0x4D;
        const uint8_t OPENED_BRACKET = 0x54;
        const uint8_t CLOSED_BRACKET = 0x5B;
        const uint8_t BACKSLASH = 0x5D;

        const uint8_t CAPS_LOCK = 0x58;
        const uint8_t A = 0x1C;
        const uint8_t S = 0x1B;
        const uint8_t D = 0x23;
        const uint8_t F = 0x2B;
        const uint8_t G = 0x34;
        const uint8_t H = 0x33;
        const uint8_t J = 0x3B;
        const uint8_t K = 0x42;
        const uint8_t L = 0x4B;
        const uint8_t SEMI_COLON = 0x4C;
        const uint8_t QUOTE = 0x52;
        const uint8_t HASH = 0x5D; // ISO hash same scancode than US blackslash
        const uint8_t ENTER = 0x5A;

        const uint8_t LEFT_SHIFT = 0x12;
        const uint8_t OPENED_ANGLE_BRACKET = 0x61;
        const uint8_t Z = 0x1A;
        const uint8_t X = 0x22;
        const uint8_t C = 0x21;
        const uint8_t V = 0x2A;
        const uint8_t B = 0x32;
        const uint8_t N = 0x31;
        const uint8_t M = 0x3A;
        const uint8_t COMMA = 0x41;
        const uint8_t DOT = 0x49;
        const uint8_t SLASH = 0x4A;
        const uint8_t RO = 0x51;
        const uint8_t RIGHT_SHIFT = 0x59;

        const uint8_t LEFT_CTRL = 0x14;
        const uint8_t LEFT_ALT = 0x11;
        const uint8_t MHEN = 0x67;
        const uint8_t SPACE = 0x29;
        const uint8_t HENK = 0x64;
        const uint8_t KANA = 0x13;

        const uint8_t NUM_LOCK = 0x77;
        const uint8_t NUM_ASTERISK = 0x7C;
        const uint8_t NUM_MINUS = 0x7B;
        const uint8_t NUM_SEVEN = 0x6C;
        const uint8_t NUM_EIGHT = 0x75;
        const uint8_t NUM_NINE = 0x7D;
        const uint8_t NUM_PLUS = 0x79;
        const uint8_t NUM_FOUR = 0x6B;
        const uint8_t NUM_FIVE = 0x73;
        const uint8_t NUM_SIX = 0x74;
        const uint8_t NUM_KP_COMMA = 0x6D;
        const uint8_t NUM_ONE = 0x69;
        const uint8_t NUM_TWO = 0x72;
        const uint8_t NUM_THREE = 0x7A;
        const uint8_t NUM_HASH = 0x68;
        const uint8_t NUM_ZERO = 0x70;
        const uint8_t NUM_DOT = 0x71;
        const uint8_t NUM_KP_EQUAL = 0x63;

        namespace extended {

            const uint8_t LEFT_OS = 0x1F | 0x80;
            const uint8_t RIGHT_ALT = 0x11 | 0x80;
            const uint8_t RIGHT_OS = 0x27 | 0x80;
            const uint8_t MENU = 0x2F | 0x80;
            const uint8_t RIGHT_CTRL = 0x14 | 0x80;

            const uint8_t UP = 0x75 | 0x80;
            const uint8_t DOWN = 0x72 | 0x80;
            const uint8_t LEFT = 0x6B | 0x80;
            const uint8_t RIGHT = 0x74 | 0x80;

            const uint8_t INSERT = 0x70 | 0x80;
            const uint8_t HOME = 0x6C | 0x80;
            const uint8_t PAGE_UP = 0x7D | 0x80;
            const uint8_t DELETE = 0x71 | 0x80;
            const uint8_t END = 0x69 | 0x80;
            const uint8_t PAGE_DOWN = 0x7A | 0x80;

            const uint8_t VOLUME_DOWN = 0x21 | 0x80;
            const uint8_t VOLUME_UP = 0x32 | 0x80;
            const uint8_t MUTE = 0x23 | 0x80;

            const uint8_t NUM_SLASH = 0x4A | 0x80;
            const uint8_t NUM_ENTER = 0x5A | 0x80;

        } // namespace extended

    } // namespace ps2

    namespace amiga {

        const uint8_t ESC = 0x45;
        const uint8_t F1 = 0x50;
        const uint8_t F2 = 0x51;
        const uint8_t F3 = 0x52;
        const uint8_t F4 = 0x53;
        const uint8_t F5 = 0x54;
        const uint8_t F6 = 0x55;
        const uint8_t F7 = 0x56;
        const uint8_t F8 = 0x57;
        const uint8_t F9 = 0x58;
        const uint8_t F10 = 0x59;

        const uint8_t BACKQUOTE = 0x00;
        const uint8_t ONE = 0x01;
        const uint8_t TWO = 0x02;
        const uint8_t THREE = 0x03;
        const uint8_t FOUR = 0x04;
        const uint8_t FIVE = 0x05;
        const uint8_t SIX = 0x06;
        const uint8_t SEVEN = 0x07;
        const uint8_t EIGHT = 0x08;
        const uint8_t NINE = 0x09;
        const uint8_t ZERO = 0x0A;
        const uint8_t HYPHEN = 0x0B;
        const uint8_t EQUAL = 0x0C;
        const uint8_t BACKSLASH = 0x0D;
        const uint8_t BACKSPACE = 0x41;

        const uint8_t TAB = 0x42;
        const uint8_t Q = 0x10;
        const uint8_t W = 0x11;
        const uint8_t E = 0x12;
        const uint8_t R = 0x13;
        const uint8_t T = 0x14;
        const uint8_t Y = 0x15;
        const uint8_t U = 0x16;
        const uint8_t I = 0x17;
        const uint8_t O = 0x18;
        const uint8_t P = 0x19;
        const uint8_t OPENED_BRACKET = 0x1A;
        const uint8_t CLOSED_BRACKET = 0x1B;

        const uint8_t CAPS_LOCK = 0x62;
        const uint8_t A = 0x20;
        const uint8_t S = 0x21;
        const uint8_t D = 0x22;
        const uint8_t F = 0x23;
        const uint8_t G = 0x24;
        const uint8_t H = 0x25;
        const uint8_t J = 0x26;
        const uint8_t K = 0x27;
        const uint8_t L = 0x28;
        const uint8_t SEMI_COLON = 0x29;
        const uint8_t QUOTE = 0x2A;
        const uint8_t ENTER = 0x44;

        const uint8_t LEFT_SHIFT = 0x60;
        const uint8_t Z = 0x31;
        const uint8_t X = 0x32;
        const uint8_t C = 0x33;
        const uint8_t V = 0x34;
        const uint8_t B = 0x35;
        const uint8_t N = 0x36;
        const uint8_t M = 0x37;
        const uint8_t COMMA = 0x38;
        const uint8_t DOT = 0x39;
        const uint8_t SLASH = 0x3A;
        const uint8_t RIGHT_SHIFT = 0x61;

        const uint8_t LEFT_CTRL = 0x63;
        const uint8_t LEFT_AMIGA = 0x66;
        const uint8_t LEFT_ALT = 0x64;
        const uint8_t SPACE = 0x40;
        const uint8_t RIGHT_ALT = 0x65;
        const uint8_t RIGHT_AMIGA = 0x67;
        const uint8_t RIGHT_CTRL = 0x63;

        const uint8_t HELP = 0x5F;
        const uint8_t DELETE = 0x46;

        const uint8_t UP = 0x4C;
        const uint8_t DOWN = 0x4D;
        const uint8_t LEFT = 0x4F;
        const uint8_t RIGHT = 0x4E;

        const uint8_t NUM_OPENED_PARENTHESIS = 0x5A;
        const uint8_t NUM_CLOSED_PARENTHESIS = 0x5B;
        const uint8_t NUM_SLASH = 0x5C;
        const uint8_t NUM_ASTERISK = 0x5D;
        const uint8_t NUM_SEVEN = 0x3D;
        const uint8_t NUM_EIGHT = 0x3E;
        const uint8_t NUM_NINE = 0x3F;
        const uint8_t NUM_PLUS = 0x5E;
        const uint8_t NUM_MINUS = 0x4A;
        const uint8_t NUM_FOUR = 0x2D;
        const uint8_t NUM_FIVE = 0x2E;
        const uint8_t NUM_SIX = 0x2F;
        const uint8_t NUM_ONE = 0x1D;
        const uint8_t NUM_TWO = 0x1E;
        const uint8_t NUM_THREE = 0x1F;
        const uint8_t NUM_ZERO = 0x0F;
        const uint8_t NUM_DOT = 0x3C;
        const uint8_t NUM_ENTER = 0x43;

    } // namespace amiga

} // namespace scancodes
