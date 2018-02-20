/*
 * Copyright (c) 2014 Steve Ward
 * Copyright (c) 2018 Jens Kerrinnes
 * LICENCE: MIT License (look at /LICENCE.md)
 */

#include <device.h>

#define PIX_HEIGHT 64L
#define PIX_WIDTH 128L

void braille_draw(uint8_t port);
void braille_clear(void);
void braille_line(int x0, int y0, int x1, int y1);
void braille_setPixel(uint8_t x, uint8_t y);