#include <stdint.h>

#ifndef SHORT_H
#define SHORT_H

uint8_t shortBIT[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C,
	0x00, 0x80, 0xFF, 0x01, 0xC0, 0x00, 0x03, 0x60,
	0x00, 0x06, 0x30, 0x00, 0x0C, 0x18, 0x00, 0x18,
	0x08, 0x00, 0x10, 0x08, 0x00, 0x10, 0x0C, 0x00,
	0x10, 0x8C, 0x9F, 0x31, 0x8C, 0x9F, 0x31, 0x0C,
	0x00, 0x10, 0x08, 0x00, 0x10, 0x08, 0x00, 0x10,
	0x18, 0x00, 0x18, 0x30, 0x00, 0x0C, 0x60, 0x00,
	0x06, 0xC0, 0x00, 0x03, 0x80, 0xFF, 0x01, 0x00,
	0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

int shortBIT_width  = 24;
int shortBIT_height = 24;

#endif
