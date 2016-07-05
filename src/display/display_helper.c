#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <Display.h>

#include "font/font_vaporware.h"

void printNumber(char *buff, int32_t temperature) {
    siprintf(buff, "%"PRIu32, temperature);
}

void getPercent(char *buff, int8_t percent) {
    siprintf(buff, "%d%%", percent);
}

void getString(char *buff, char *state) {
    siprintf(buff, "%s", state);
}

void getFloating(char *buff, int32_t floating) {
    siprintf(buff, "%"PRIu32".%02"PRIu32, floating / 1000, floating % 1000 / 10);
}

void getFloatingTenth(char *buff, uint32_t floating) {
    siprintf(buff, "%"PRIu32".%01"PRIu32, floating / 1000, floating % 1000 / 100);
}

void buildRow(uint8_t y, uint8_t* icon, void (*parsingCallback)(char* buff, int32_t value), uint32_t value) {
    char buff[9];
    Display_PutPixels(0, y, icon, 24, 24);

    parsingCallback(buff, value);
    Display_PutText(26, y+5, buff, FONT_MEDIUM);
}

void buildItem(uint8_t x, uint8_t y, uint8_t x2, uint8_t y2, uint8_t* icon, void (*parsingCallback)(char* buff, int32_t value), uint32_t value){
  char buff[9];
  Display_PutPixels(x,y,icon, 24, 24);

  parsingCallback(buff, value);
  Display_PutText(x2, y2, buff, FONT_MEDIUM);
}

/* Will always show 3 decimals, todo: make the '3' a param */
void formatFixedPoint(int32_t value, int32_t divisor, char *formatted) {
    if(divisor == 0)
        siprintf(formatted, "infin");
    else
        siprintf(formatted, "%"PRId32".%03"PRId32, value/divisor, value % divisor);
}

void formatThousandths(char *formatted, int32_t value) {
    formatFixedPoint(value, 1000, formatted);
}

void formatINT(int32_t value, char *formatted) {
	printNumber(formatted, value);
}
