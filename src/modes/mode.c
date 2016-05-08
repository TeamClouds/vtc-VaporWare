#include "globals.h"
#include "mode.h"
#include "settings.h"

void setVapeMode(int newMode) {
    if (newMode >= modeCount)
        return;

    s.mode = newMode;

    __vape = g.vapeModes[newMode]->fire;
    __up = g.vapeModes[newMode]->increase;
    __down = g.vapeModes[newMode]->decrease;
    if (g.vapeModes[newMode]->init) {
        __init = g.vapeModes[newMode]->init;
        __init();
    }
}