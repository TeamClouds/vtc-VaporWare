#ifndef __BUTTON_H
#define __BUTTON_H

#include <stdint.h>

enum {
	FIRE_HOLD_EVENT = 1,
	LEFT_HOLD_EVENT = 2,
	RIGHT_HOLD_EVENT = 4,
	FIRE_REPEAT = 8,
	LEFT_REPEAT = 16,
	RIGHT_REPEAT = 32
};

enum {
    BUTTON_REL = 0,
    BUTTON_PRESS = 1,
    BUTTON_HELD = 2,
    BUTTON_REPEAT = 4
};

struct buttonHandler {
    const char *name;

    void (*fire_handler)(uint8_t state, uint32_t duration);
    void (*left_handler)(uint8_t state, uint32_t duration);
    void (*right_handler)(uint8_t state, uint32_t duration);
    uint16_t flags;
    
    void (*fire_repeated)(void);
    uint32_t fireRepeatTimeout;
    uint8_t fireRepeatCount;
    uint32_t fireUpdateInterval;
    
    void (*left_repeated)(void);
    uint32_t leftRepeatTimeout;
    uint8_t leftRepeatCount;
    uint32_t leftUpdateInterval;

    void (*right_repeated)(void);
    uint32_t rightRepeatTimeout;
    uint8_t rightRepeatCount;
    uint32_t rightUpdateInterval;

    struct buttonHandler *stashedHandler;
};

void initHandlers();
void freeHandlers();

void setHandler(struct buttonHandler *b);
void switchHandler(struct buttonHandler *b);
void returnHandler();

volatile uint32_t *buttonTimeout;
void buttonTimer(uint32_t ignored);
void handleButtonEvents();
#endif