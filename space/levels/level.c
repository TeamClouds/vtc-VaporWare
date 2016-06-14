#include <stdlib.h>
#include <Display.h>
#include <variabletimer.h>
#include <stdio.h>

#include "game_globals.h"
#include "shot.h"
#include "globals.h"
#include "level.h"

char buff[63];

uint8_t progressShot(shot *s) {
    // are shot bounds in screen?
    if (s->y >= 0) {
        // drive the shot up if it's time to move it
        if (s->lastAnimStep + SHOT_STEP_INTERVAL < uptime) {
            s->lastAnimStep = uptime;
            if (s->direction == 0) {
                s->y -= 1;
            } else if (s->direction == 1) {
                s->y += 1;
            }
        }
    }

    if (clearShotIfOutOfBounds(s)) {
       return SHOT_OUT_OF_BOUNDS;
    }
    return 0;
}

bool clearShotIfOutOfBounds(shot *s) {
    // check if we're out of bounds now
    if (s->y <= 0 || s->y >= DISPLAY_HEIGHT) {
        clearShot(s);
        return true;
    }
    return false;
}

void clearShot(shot *s) {
    // clear shot for use
    s->x = -1;
    s->y = -1;
    s->lastAnimStep = 0;
}

void setLevel(int newLevel) {
    if (newLevel >= gg.levelCount)
        return;

    __init = gg.levels[newLevel]->init;
    __draw = gg.levels[newLevel]->draw;
    __progress = gg.levels[newLevel]->progress;
    if (gg.levels[newLevel]->init) {
        __init = gg.levels[newLevel]->init;
        __init();
    }

    for (int i = 0; i < SHOTS; i++) {
        gg.shots[i].lastAnimStep = 0;
        gg.shots[i].x = -1;
        gg.shots[i].y = -1;
    }
    gg.xEnemyOffset = 0;
    gg.yEnemyOffset = 0;
    gg.lastAlienShiftTime = 0;

    Display_Clear();
    Display_SetInverted(true);
    siprintf(buff, "Level:%d", newLevel);
    Display_PutText(0, 50, buff, FONT_DEJAVU_8PT);
    Display_Update();

    uint32_t now = uptime;
    while (now + 500 > uptime) {
        // wait
    }
    Display_SetInverted(false);
    Display_Update();
    while (now + 500 > uptime) {
        // wait
    }
    Display_Clear();
}

void addLevel(struct level *lvl) {
    if (gg.levels == NULL) {
        gg.levelCount = lvl->index + 1;
        gg.levels = (struct level **)malloc(gg.levelCount * sizeof(struct level *));

    } else {
        if (lvl->index + 1 > gg.levelCount) {
            gg.levelCount = lvl->index + 1;
            gg.levels = (struct level **)realloc(gg.levels, gg.levelCount * sizeof(struct level *));
        }
    }
    gg.levels[lvl->index] = lvl;
}

struct level LEVELMAX = {
        .index = LEVELMAXCONTROL
};

static void __attribute__((constructor)) registerSentinelLevel(void) {
    addLevel(&LEVELMAX);
}