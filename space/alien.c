#include <Display.h>
#include <variabletimer.h>
#include "game_globals.h"
#include "alien.h"

bool isAlienFiring(alien *a) {
    return a->shots[0].y < DISPLAY_HEIGHT && a->shots[0].y > 0 && a->shots[0].lastAnimStep != 0;
}

void alienFire(alien *a) {
    if (!isAlienFiring(a)) {
        a->shots[0].x = a->x + (((a->w * 100) / 2) / 100) + gg.xEnemyOffset;
        a->shots[0].y = a->y + SHOT_LEN + gg.xEnemyOffset;
        a->shots[0].lastAnimStep = uptime;
        a->shots[0].direction = 1;
    }
}