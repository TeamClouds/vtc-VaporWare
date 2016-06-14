#ifndef __GAME_GLOBALS_H
#define __GAME_GLOBALS_H

#include <stdbool.h>
#include <stdint.h>

#include "alien.h"
#include "level.h"
#include "shot.h"

#define SHOTS 10

struct game_globals {
    struct level **levels;
    uint8_t levelCount;
    uint8_t userScore;

    uint8_t score;
    uint8_t playerHealth;
    uint8_t maxPlayerHealth;
    uint8_t alienDirection;
    uint32_t lastAlienShiftTime;
    shot shots[SHOTS];
    int8_t shipCoords[2];
    uint8_t xEnemyOffset;
    uint8_t yEnemyOffset;
};

bool isAlienShot(alien *enemy);

bool isShipShot(shot *shoost, int8_t shipX, int8_t shipY, uint8_t shipW, uint8_t shipH);

bool isIntersecting(int8_t x1, uint8_t w1, int8_t y1, uint8_t h1,
                    int8_t x2, int8_t y2);

extern struct game_globals gg;

#endif
