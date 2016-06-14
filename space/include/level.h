#ifndef _LEVEL_H
#define _LEVEL_H

#include <stdint.h>
#include <stdbool.h>
#include "shot.h"

#define SHOT_OUT_OF_BOUNDS 5

uint8_t progressShot(shot *s);
bool clearShotIfOutOfBounds(shot *s);
void clearShot(shot *s);

enum {
    LEVEL1,
    LEVEL2,
    LEVELMAXCONTROL
};

struct level {
    int8_t index;
    int8_t controlType;
    uint8_t numAliens;
    volatile uint8_t aliveAliens;
    void (*init) (void);
    void (*progress) (void);
    void (*draw) (void);
};

void addLevel(struct level *lvl);
void setLevel(int level);

void (*__init) (void);
void (*__progress) (void);
void (*__draw) (void);

#endif //_LEVEL_H
