#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <Display.h>

#include "font/font_vaporware.h"

void printNumber(char *buff, uint8_t len, int32_t temperature) {
    sniprintf(buff, len, "%"PRIu32, temperature);
}

void getPercent(char *buff, uint8_t len, int8_t percent) {
    sniprintf(buff, len, "%d%%", percent);
}

void getString(char *buff, uint8_t len, char *state) {
    sniprintf(buff, len, "%s", state);
}

void getFloating(char *buff, uint8_t len, int32_t floating) {
    sniprintf(buff, len, "%"PRIu32".%02"PRIu32, floating / 1000, floating % 1000 / 10);
}

void getFloatingTenth(char *buff, uint8_t len, uint32_t floating) {
    sniprintf(buff, len, "%"PRIu32".%01"PRIu32, floating / 1000, floating % 1000 / 100);
}

void buildRow(uint8_t y, uint8_t* icon, void (*parsingCallback)(char* buff, uint8_t len, int32_t value), uint32_t value) {
    char buff[9];
    Display_PutPixels(0, y, icon, 24, 24);

    parsingCallback(buff, 9, value);
    Display_PutText(26, y+5, buff, FONT_MEDIUM);
}

/* Will always show 3 decimals, todo: make the '3' a param */
void formatFixedPoint(int32_t value, int32_t divisor, char *formatted, uint8_t len) {
    if(divisor == 0)
        sniprintf(formatted, len, "infin");
    else
        sniprintf(formatted, len, "%"PRId32".%03"PRId32, value/divisor, value % divisor);
}

void formatThousandths(char *formatted, uint8_t len, int32_t value) {
    formatFixedPoint(value, 1000, formatted, len);
}

void formatINT(int32_t value, char *formatted, uint8_t len) {
	printNumber(formatted, len, value);
}
