#include <stdlib.h>

#include "globals.h"
#include "mode.h"
#include "settings.h"

void setVapeMode(int newMode) {
    if (newMode >= g.modeCount)
        return;

    __vape = g.vapeModes[newMode]->fire;
    __up = g.vapeModes[newMode]->increase;
    __down = g.vapeModes[newMode]->decrease;
    if (g.vapeModes[newMode]->init) {
        __init = g.vapeModes[newMode]->init;
        __init();
    }
}

void addVapeMode(struct vapeMode *vm) {
    if (g.vapeModes == NULL) {
        g.modeCount = vm->index + 1;
        g.vapeModes = (struct vapeMode **)malloc(g.modeCount * sizeof(struct vapemode *));

    } else {
        if (vm->index + 1 > g.modeCount) {
            g.modeCount = vm->index + 1;
            g.vapeModes = (struct vapeMode **)realloc(g.vapeModes, g.modeCount * sizeof(struct vapeMode *));
        }
    }

    g.vapeModes[vm->index] = vm;
}

struct vapeMode THEMAX = {
        .name = "\0",
        .index = MAX_CONTROL,
};

static void __attribute__((constructor)) registerSentinelMode(void) {
    addVapeMode(&THEMAX);
}
