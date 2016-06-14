#include <stdbool.h>
#include "game_globals.h"
#include "shot.h"
#include "alien.h"

struct game_globals gg = {
        .levelCount = 3,
        .playerHealth = 3,
        .maxPlayerHealth = 3,
        .userScore = 0,
        .alienDirection = 1,
        .lastAlienShiftTime = 0
};


bool isIntersecting(int8_t x1, uint8_t w1, int8_t y1, uint8_t h1,
                    int8_t x2, int8_t y2) {
    return (x2 >= x1 && x2 <= x1 + w1) && (y2 >= y1 && y2 <= y1 + h1);
}

bool isShipShot(shot *shoost, int8_t shipX, int8_t shipY, uint8_t shipW, uint8_t shipH) {
    return isIntersecting(shipX, shipW, shipY, shipH, shoost->x, shoost->y);
}

bool isAlienShot(alien *enemy) {
    for (uint8_t i = 0; i < SHOTS; i++) {
        if (gg.shots->lastAnimStep > 0 && isIntersecting(enemy->x + gg.xEnemyOffset,
                                                        enemy->w, enemy->y + gg.yEnemyOffset,
                                                        enemy->h, gg.shots[i].x, gg.shots[i].y)) {
            // eat shot
            gg.shots[i].x = -1;
            gg.shots[i].y = -1;
            gg.shots[i].lastAnimStep = 0;

            return true;
        }
    }
    return false;
}