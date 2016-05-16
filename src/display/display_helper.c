#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <Display.h>

#include "font/font_vaporware.h"

void printNumber(char *buff, uint32_t temperature) {
    siprintf(buff, "%" PRIu32, temperature);
}

void getPercent(char *buff, uint8_t percent) {
    siprintf(buff, "%d%%", percent);
}

void getString(char *buff, char *state) {
    siprintf(buff, "%s", state);
}

void getFloating(char *buff, uint32_t floating) {
    siprintf(buff, "%"PRIu32".%02"PRIu32, floating / 1000, (floating % 1000)/10);
}

void getFloatingTenth(char *buff, uint32_t floating) {
    siprintf(buff, "%"PRIu32".%01"PRIu32, floating / 1000, floating % 1000 / 100);
}

void buildRow(uint8_t y, uint8_t* icon, void (*parsingCallback)(char* buff, uint32_t value), uint32_t value) {
    char buff[9];
    Display_PutPixels(0, y, icon, 24, 24);

    parsingCallback(buff, value);
    Display_PutText(26, y+5, buff, FONT_MEDIUM);
}
