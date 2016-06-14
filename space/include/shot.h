#ifndef __SHOT_H
#define __SHOT_H

#include <stdint.h>

#define SHOT_LEN 3
#define SHOT_STEP_INTERVAL 25

typedef struct {
    int8_t x, y;
    uint8_t direction /* 0 =down, 1 = up*/;
    uint32_t lastAnimStep;
} shot;

#endif //__SHOT_H
