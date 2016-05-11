#include <stdio.h>
#include <string.h>
#include <stdint.h>

void printNumber(char *buff, uint32_t temperature) {
    siprintf(buff, "%lu", temperature);
}

void getPercent(char *buff, uint8_t percent) {
    siprintf(buff, "%d%%", percent);
}

void getString(char *buff, char *state) {
    siprintf(buff, "%s", state);
}

void getFloating(char *buff, uint32_t floating) {
    siprintf(buff, "%lu.%02lu", floating / 1000, floating % 1000 / 10);
}

void getFloatingTenth(char *buff, uint32_t floating) {
    siprintf(buff, "%lu.%01lu", floating / 1000, floating % 1000 / 100);
}
