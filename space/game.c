#include <M451Series.h>
#include <Display.h>
#include <stdio.h>
#include <variabletimer.h>


#include "globals.h"
#include "game_globals.h"
#include "button.h"
#include "space_res.h"

#define HEADER_HEIGHT 12

int8_t currentLevel;
uint32_t lastFireTime;

/**
 * return next available shot slot
 */
int findNextShotIdx() {
    for (int i = 0; i < SHOTS; i++) {
        if (gg.shots[i].lastAnimStep == 0) {
            return i;
        }
    }
    return -1;
}

void moveLeft(uint8_t status, uint32_t held) {
    int amount = 0;
    if ((status & BUTTON_PRESS) && (held < 300)) {
        amount = 4;
    } else if ((held > 300) && status & BUTTON_HELD) {
        amount = 2;
    }
    if (amount > 0) {
        // check bounds
        if ((gg.shipCoords[0] - amount) >= 0) {
            gg.shipCoords[0] -= amount;
        }
    }
}

void moveRight(uint8_t status, uint32_t held) {
    int amount = 0;
    if ((status & BUTTON_PRESS) && (held < 300)) {
        amount = 4;
    } else if ((held > 300) && status & BUTTON_HELD) {
        amount = 2;
    }
    if (amount > 0) {
        // check bounds
        if ((gg.shipCoords[0] + amount) < (DISPLAY_WIDTH - Bitmap_ship_width)) {
            gg.shipCoords[0] += amount;
        }
    }
}

void shoost(uint8_t status, uint32_t held) {
    if ((status & BUTTON_PRESS) && lastFireTime + 250 < uptime) {
        lastFireTime = uptime;
        int nextSlot = findNextShotIdx();
        if (nextSlot >= 0) {
            gg.shots[nextSlot].x = gg.shipCoords[0] + ((Bitmap_ship_width / 2));
            gg.shots[nextSlot].y = DISPLAY_HEIGHT - Bitmap_ship_height;
            gg.shots[nextSlot].lastAnimStep = uptime;
            gg.shots[nextSlot].direction = 0;
        }
    }
}


struct buttonHandler gameHandler = {
        .name = "mainButtons",
        .flags = LEFT_HOLD_EVENT | RIGHT_HOLD_EVENT,

        .fire_handler = &shoost,
        .fireUpdateInterval = 500,

        .left_handler = &moveLeft,
        .leftUpdateInterval = 50,

        .right_handler = &moveRight,
        .rightUpdateInterval = 50,

};

char buff[63];

void draw() {
    Display_Clear();

    // draw header
    siprintf(buff, "Score:%d", gg.userScore);
    Display_PutText(0, 0, buff, FONT_DEJAVU_8PT);

    Display_PutLine(0, HEADER_HEIGHT, DISPLAY_WIDTH, HEADER_HEIGHT);

    gg.yEnemyOffset = HEADER_HEIGHT + 4;

    // ship
    Display_PutPixels(gg.shipCoords[0], gg.shipCoords[1], Bitmap_ship, Bitmap_ship_width, Bitmap_ship_height);

    // display health
    uint8_t healthBarWidth = ((DISPLAY_WIDTH * 100) / gg.maxPlayerHealth) / 100;
    uint8_t x = 0;
    healthBarWidth -= 4; // shave off two pixels from each side for separation
    for (int i = 0; i < gg.playerHealth; i++) {
        x += 2;
        Display_PutLine(x, DISPLAY_HEIGHT - 2, x + healthBarWidth, DISPLAY_HEIGHT - 2);
        x += healthBarWidth + 2;
    }

    // TODO maybe move shot rendering there too to give it full control?
    gg.levels[currentLevel]->draw();

    Display_Update();
}

int progress() {
    gg.levels[currentLevel]->progress();
    return 0;
}

void runSpace() {
    switchHandler(&gameHandler);

    // set ship loc
    gg.shipCoords[0] = (DISPLAY_WIDTH / 2) - (Bitmap_ship_width / 2);
    gg.shipCoords[1] = DISPLAY_HEIGHT - Bitmap_ship_height - 4 /* some padding */;

    setLevel(currentLevel = 1);
    progress();
    draw();

    while (1) {
        if (gg.playerHealth == 0) {
            Display_Clear();
            Display_PutText(0, 50, "GameOver", FONT_DEJAVU_8PT);
            Display_Update();

            uint32_t sleep = uptime + 1000;
            while (sleep > uptime) {
                // wait
            }

            Display_Clear();
            gv.spacinVaper = 0;
            returnHandler();
            return;
        }
        if (gg.levels[currentLevel]->aliveAliens == 0) {
            setLevel(++currentLevel);
        }

        if (gv.buttonEvent) {
            handleButtonEvents();
            gv.buttonEvent = 0;
        }


        progress();
        draw();
    }
}
