#ifndef _ALIEN_H
#define _ALIEN_H

#include <stdbool.h>
#include "shot.h"

typedef struct {
    uint8_t x, y;
    uint8_t w, h;
    int8_t health;
    uint32_t lastHitTime;
    uint8_t *moveAnimations[2];
    shot shots[1];
} alien;

bool isAlienFiring(alien *a);
void alienFire(alien *a);

#endif //_ALIEN_H
