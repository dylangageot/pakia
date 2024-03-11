#pragma once

#include <avr/pgmspace.h>

const uint8_t ps2_scancode_to_amiga_keycode[256] PROGMEM = {
    0x00, 0x58, 0x00, 0x54, 0x52, 0x50, 0x51, 0x00, 
    0x00, 0x59, 0x57, 0x55, 0x53, 0x42, 0x00, 0x00, 
    0x00, 0x64, 0x60, 0x00, 0x63, 0x10, 0x01, 0x00, 
    0x00, 0x00, 0x31, 0x21, 0x20, 0x11, 0x02, 0x00, 
    0x00, 0x33, 0x32, 0x22, 0x12, 0x04, 0x03, 0x00, 
    0x00, 0x40, 0x34, 0x23, 0x14, 0x13, 0x05, 0x00, 
    0x00, 0x36, 0x35, 0x25, 0x24, 0x15, 0x06, 0x00, 
    0x00, 0x00, 0x37, 0x26, 0x16, 0x07, 0x08, 0x00, 
    0x00, 0x38, 0x27, 0x17, 0x18, 0x0a, 0x09, 0x00, 
    0x00, 0x39, 0x3a, 0x28, 0x29, 0x19, 0x0b, 0x00, 
    0x00, 0x00, 0x2a, 0x00, 0x1a, 0x0c, 0x00, 0x00, 
    0x62, 0x61, 0x44, 0x1b, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x41, 0x00, 
    0x00, 0x1d, 0x00, 0x2d, 0x3d, 0x00, 0x00, 0x00, 
    0x0f, 0x3c, 0x1e, 0x2e, 0x2f, 0x3e, 0x45, 0x5a, 
    0x5f, 0x5e, 0x1f, 0x4a, 0x5d, 0x3f, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x56, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x65, 0x00, 0x00, 0x63, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x67, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x5c, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x4f, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x46, 0x4d, 0x00, 0x4e, 0x4c, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};