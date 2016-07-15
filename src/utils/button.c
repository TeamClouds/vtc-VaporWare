#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <Button.h>
#include <TimerUtils.h>

#include "button.h"
#include "globals.h"
#include "variabletimer.h"

struct buttonGlobals {
    volatile uint32_t fireStart;
    volatile uint32_t fireNext;
    volatile uint32_t fireTimeout;
    volatile uint8_t fireHeld;
    volatile uint8_t fireCount;
    volatile uint8_t callFireCallback;
    volatile uint8_t callFireRepeatCallback;
    
    volatile uint32_t leftStart;
    volatile uint32_t leftNext;
    volatile uint32_t leftTimeout;
    volatile uint8_t leftHeld;
    volatile uint8_t leftCount;
    volatile uint8_t callLeftCallback;
    volatile uint8_t callLeftRepeatCallback;

    volatile uint32_t rightStart;
    volatile uint32_t rightNext;
    volatile uint32_t rightTimeout;
    volatile uint8_t rightHeld;
    volatile uint8_t rightCount;
    volatile uint8_t callRightCallback;
    volatile uint8_t callRightRepeatCallback;

    volatile uint32_t buttonTimerExpires;

    /* These variables need to be persisted across state clears */
    uint8_t buttonHandlerIndex;
    struct buttonHandler *currentHandler;
} bg = {0};


void handleButtonEvents() {
    struct buttonHandler *b = bg.currentHandler;
    uint32_t ltime = uptime;
    
    if (bg.callFireCallback) {
        b->fire_handler(bg.fireHeld, ltime - bg.fireStart);
        bg.callFireCallback = 0;
    }
    if (bg.callFireRepeatCallback) {
        b->fire_repeated();
        bg.callFireRepeatCallback = 0;
    }

    if (bg.callLeftCallback) {
        b->left_handler(bg.leftHeld, ltime - bg.leftStart);
        bg.callLeftCallback = 0;
    }
    if (bg.callLeftRepeatCallback) {
        b->left_repeated();
        bg.callLeftRepeatCallback = 0;
    }
    
    if (bg.callRightCallback) {
        b->right_handler(bg.rightHeld, ltime - bg.rightStart);
        bg.callRightCallback = 0;
    }
    if (bg.callRightRepeatCallback) {
        b->right_repeated();
        bg.callRightRepeatCallback = 0;
    }
    

}

void buttonTimer(uint32_t ignored) {
    uint32_t ltime = uptime;
    
    /* Fire button */
    /* Periodic callback */
    if (bg.fireNext && ltime >= bg.fireNext && bg.fireHeld) {
        bg.callFireCallback = 1;
        bg.fireHeld = BUTTON_HELD;
        bg.fireNext = ltime + bg.currentHandler->fireUpdateInterval;
        bg.buttonTimerExpires = ltime + 5000;
        gv.buttonEvent = 1;
    }

    /* Repeated Press callback */
    if (bg.currentHandler->fireRepeatCount && 
        bg.fireCount >= bg.currentHandler->fireRepeatCount) {
        bg.callFireRepeatCallback = 1;
        bg.fireHeld = BUTTON_REPEAT;
        bg.fireCount = 0;
        gv.buttonEvent = 1;
    }

    if (bg.fireTimeout && ltime >= bg.fireTimeout) {
        bg.fireCount = 0;
        if (bg.fireHeld) {
            bg.callFireCallback = 1;
            gv.buttonEvent = 1;
        }
    }

    /* Left Button */
    if (bg.leftNext && ltime >= bg.leftNext && bg.leftHeld) {
        bg.callLeftCallback = 1;
        bg.leftHeld = BUTTON_HELD;
        bg.leftNext = ltime + bg.currentHandler->leftUpdateInterval;
        bg.buttonTimerExpires = ltime + 5000;
        gv.buttonEvent = 1;
    }

    if (bg.currentHandler->leftRepeatCount &&
        bg.leftCount >= bg.currentHandler->leftRepeatCount) {
        bg.callLeftRepeatCallback = 1;
        bg.leftHeld = BUTTON_REPEAT;
        bg.leftCount = 0;
        gv.buttonEvent = 1;
    }

    if (bg.leftTimeout && ltime >= bg.leftTimeout) {
        bg.leftCount = 0;
        if (bg.leftHeld) {
            bg.callLeftCallback = 1;
            gv.buttonEvent = 1;
        }
    }

    /* Right Button */
    if (bg.rightNext && ltime >= bg.rightNext && bg.rightHeld) {
        bg.callRightCallback = 1;
        bg.rightHeld = BUTTON_HELD;
        bg.rightNext = ltime + bg.currentHandler->rightUpdateInterval;
        bg.buttonTimerExpires = ltime + 5000;
        gv.buttonEvent = 1;
    }

    if (bg.currentHandler->rightRepeatCount &&
        bg.rightCount >= bg.currentHandler->rightRepeatCount) {
        bg.callRightRepeatCallback = 1;
        bg.rightHeld = BUTTON_REPEAT;
        bg.rightCount = 0;
        gv.buttonEvent = 1;
    }

    if (bg.rightTimeout && ltime >= bg.rightTimeout) {
        bg.rightCount = 0;
        if (bg.rightHeld) {
            bg.callRightCallback = 1;
            gv.buttonEvent = 1;
        }
    }
}

void buttonPressed(uint8_t state) {
    uint32_t ltime = uptime;

    if (state & BUTTON_MASK_FIRE) {
        gv.fireButtonPressed = 1;
        if (!bg.fireHeld) {
            bg.fireHeld = BUTTON_PRESS;
            bg.callFireCallback = 1;
            if (bg.currentHandler->flags & FIRE_HOLD_EVENT) {
                bg.fireStart = ltime;
                bg.fireNext = ltime + bg.currentHandler->fireUpdateInterval;
            }
            if (bg.currentHandler->flags & FIRE_REPEAT) {
                bg.fireCount++;
                bg.callFireCallback = 0;
                bg.fireTimeout = ltime + bg.currentHandler->fireRepeatTimeout;
            }
        }
    } else if (bg.fireHeld && !(state & BUTTON_MASK_FIRE)) {
        bg.callFireCallback = 1;
        
        gv.fireButtonPressed = 0;
        bg.fireStart = 0;
        bg.fireNext = 0;
        bg.fireHeld = BUTTON_REL;
    }

    if (state & BUTTON_MASK_LEFT) {

        if (!bg.leftHeld) {
            bg.leftHeld = BUTTON_PRESS;
            bg.callLeftCallback = 1;
            if (bg.currentHandler->flags & LEFT_HOLD_EVENT) {
                bg.leftStart = ltime;
                bg.leftNext = ltime + bg.currentHandler->leftUpdateInterval;
            }
            if (bg.currentHandler->flags & LEFT_REPEAT) {
                bg.leftCount++;
                bg.callLeftCallback = 0;
                bg.leftTimeout = ltime + bg.currentHandler->leftRepeatTimeout;
            }
        }
    } else if (bg.leftHeld && !(state & BUTTON_MASK_LEFT)) {
        bg.callLeftCallback = 1;
       
        bg.leftStart = 0;
        bg.leftNext = 0;
        bg.leftHeld = BUTTON_REL;
    }

    if (state & BUTTON_MASK_RIGHT) {
        if (!bg.rightHeld) {
            bg.rightHeld = BUTTON_PRESS;
            bg.callRightCallback = 1;
            if (bg.currentHandler->flags & RIGHT_HOLD_EVENT) {
                bg.rightStart = ltime;
                bg.rightNext = ltime + bg.currentHandler->rightUpdateInterval;
            }
            
            if (bg.currentHandler->flags & RIGHT_REPEAT) {
                bg.rightCount++;
                bg.callRightCallback = 0;
                bg.rightTimeout = ltime + bg.currentHandler->rightRepeatTimeout;
            }
        }
    } else if (bg.rightHeld && !(state & BUTTON_MASK_RIGHT)) {
        bg.callRightCallback = 1;
        
        bg.rightStart = 0;
        bg.rightNext = 0;
        bg.rightHeld = BUTTON_REL;
    }

    gv.buttonEvent = 1;
    bg.buttonTimerExpires = ltime + 5000;

}

void initHandlers() {
    buttonTimeout = &bg.buttonTimerExpires;
    bg.buttonHandlerIndex = Button_CreateCallback(buttonPressed, 
        BUTTON_MASK_FIRE | BUTTON_MASK_RIGHT | BUTTON_MASK_LEFT);
}

void freeHandlers() {
    bg.buttonTimerExpires = 0;
    Button_DeleteCallback(bg.buttonHandlerIndex);
    bg.currentHandler = NULL;
}

void dummyHandler(uint8_t a, uint32_t b) {}
void dummyRepeatHandler() {}

void cleanupVariables() {
    uint8_t temp_buttonHandlerIndex = bg.buttonHandlerIndex;
    struct buttonHandler *temp_currentHandler = bg.currentHandler;
    memset(&bg, 0, sizeof(struct buttonGlobals));
    bg.buttonHandlerIndex = temp_buttonHandlerIndex;
    bg.currentHandler = temp_currentHandler;
}

void validateHandlers(struct buttonHandler *b) {
    if (b->fire_handler == NULL)
        b->fire_handler = &dummyHandler;
    if (b->left_handler == NULL)
        b->left_handler = &dummyHandler;
    if (b->right_handler == NULL)
        b->right_handler = &dummyHandler;
    if (b->fire_repeated == NULL)
        b->fire_repeated = &dummyRepeatHandler;
    if (b->left_repeated == NULL)
        b->left_repeated = &dummyRepeatHandler;
    if (b->right_repeated == NULL)
        b->right_repeated = &dummyRepeatHandler;
}

void setHandler(struct buttonHandler *b) {
    cleanupVariables();
    bg.currentHandler = b;
    validateHandlers(bg.currentHandler);
}

void switchHandler(struct buttonHandler *b) {
    cleanupVariables();
    b->stashedHandler = bg.currentHandler;
    bg.currentHandler = b;
    validateHandlers(bg.currentHandler);
}

void returnHandler(void) {
    cleanupVariables();
    struct buttonHandler *tempHandler = bg.currentHandler;
    bg.currentHandler = bg.currentHandler->stashedHandler;
    tempHandler->stashedHandler = NULL;
    validateHandlers(bg.currentHandler);
}
