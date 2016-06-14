#include <Display.h>
#include <variabletimer.h>
#include <alien.h>
#include <shot.h>

#include "game_globals.h"
#include "space_res.h"

#define ALIEN_DESTRUCTION_DURATION 1000
#define ALIEN_STEP_INTERVAL 300
#define ALIENS_PER_ROW 2
#define ALIEN_ROWS 5
#define ALIEN_COUNT ALIENS_PER_ROW * ALIEN_ROWS

alien aliens[ALIEN_ROWS][ALIENS_PER_ROW];

void init2();

void progress2();

void draw2();

struct level level2 = {
        .index = 2,
        .controlType = LEVEL2,
        .numAliens = ALIEN_COUNT,
        .aliveAliens = ALIEN_COUNT,
        .init = &init2,
        .progress = &progress2,
        .draw = &draw2
};

void init2() {
    uint8_t x = 0, y = 16, xSpacing = 8, ySpacing = 6;

    // setup aliens
    for (int i = 0; i < ALIEN_ROWS; i++) {
        for (int j = 0; j < ALIENS_PER_ROW; j++) {
            aliens[i][j].x = x;
            aliens[i][j].y = y;
            aliens[i][j].w = Bitmap_alien1_width;
            aliens[i][j].h = Bitmap_alien1_height;
            aliens[i][j].health = 1;
            aliens[i][j].lastHitTime = 0;
            aliens[i][j].moveAnimations[0] = Bitmap_alien1;
            aliens[i][j].moveAnimations[1] = Bitmap_alien1_step;

            x += aliens[i][j].w;

            // can we fit another on this row?
            if (x + aliens[i][j].w + xSpacing <= DISPLAY_WIDTH && j != ALIENS_PER_ROW - 1) {
                x += xSpacing;
            } else {
                x = 0;
                y += aliens[i][j].h + ySpacing;
            }
        }
    }
    gg.alienDirection = 1;
    gg.lastAlienShiftTime = 0;
}

uint8_t current_offset;
uint8_t alien2Shots = 0;

void progress2() {
    uint32_t fi = uptime % ALIENS_PER_ROW;
    uint32_t fj = uptime % ALIENS_PER_ROW;
    // register shots
    for (int i = ALIEN_ROWS - 1; i >= 0; i--) {
        for (int j = 0; j < ALIENS_PER_ROW; j++) {
            if (aliens[i][j].health > 0 && isAlienShot((&aliens[i][j]))) {
                aliens[i][j].health--;
                aliens[i][j].lastHitTime = uptime;
                if (aliens[i][j].health == 0) {
                    level2.aliveAliens--;
                    gg.userScore++;
                }
            }
            if (isAlienFiring(&aliens[i][j])) {
                if (progressShot(&aliens[i][j].shots[0]) == SHOT_OUT_OF_BOUNDS) {
                    alien2Shots--;
                } else if (isShipShot(&aliens[i][j].shots[0], gg.shipCoords[0], gg.shipCoords[1],
                                      Bitmap_ship_width, Bitmap_ship_height)) {
                    alien2Shots--;
                    gg.playerHealth--;
                    clearShot(&aliens[i][j].shots[0]);
                }
            } else if (aliens[i][j].health > 0 && fi == i && fj == j && alien2Shots < 3) {
                alien2Shots++;
                alienFire(&aliens[i][j]);
            }
        }
    }

    // progress shots
    for (int i = 0; i < SHOTS; i++) {
        progressShot(&gg.shots[i]);
    }

    // move the aliens
    if (uptime - gg.lastAlienShiftTime > ALIEN_STEP_INTERVAL) {
        // update their locs for next time
        if (gg.alienDirection == 1) {
            for (int i = 0; i < ALIEN_ROWS; i++) {
                for (int j = 0; j < ALIENS_PER_ROW; j++) {
                    current_offset += 5;
                    aliens[i][j].x += 5;
                    if (current_offset >= 10 && gg.alienDirection != 0) {
                        gg.alienDirection = 0;
                    }
                }
            }
        } else {
            for (int i = 0; i < ALIEN_ROWS; i++) {
                for (int j = 0; j < ALIENS_PER_ROW; j++) {
                    current_offset -= 5;
                    aliens[i][j].x -= 5;
                    if (current_offset <= 0 && gg.alienDirection != 1) {
                        gg.alienDirection = 1;
                    }
                }
            }
        }
        gg.lastAlienShiftTime = uptime;
    }
}

void draw2() {
    // render aliens
    for (int i = 0; i < ALIEN_ROWS; i++) {
        for (int j = 0; j < ALIENS_PER_ROW; j++) {
            if (aliens[i][j].health > 0) {
                // two-step
                Display_PutPixels(aliens[i][j].x,
                                  aliens[i][j].y,
                                  aliens[i][j].moveAnimations[gg.xEnemyOffset % 5 == 0 ? 1 : 0],
                                  aliens[i][j].w,
                                  aliens[i][j].h);
            } else if (aliens[i][j].health == 0 &&
                       aliens[i][j].lastHitTime + ALIEN_DESTRUCTION_DURATION > uptime) {
                // explosion animation
                Display_PutPixels(aliens[i][j].x,
                                  aliens[i][j].y,
                                  Bitmap_alien1_shot,
                                  Bitmap_alien1_shot_width,
                                  Bitmap_alien1_shot_height);
            }
            if (isAlienFiring(&aliens[i][j])) {
                Display_PutLine(aliens[i][j].shots[0].x,
                                aliens[i][j].shots[0].y,
                                aliens[i][j].shots[0].x,
                                aliens[i][j].shots[0].y+SHOT_LEN);
            }
        }
    }

    // render shots
    for (int i = 0; i < SHOTS; i++) {
        // are shot bounds in screen?
        Display_PutLine(gg.shots[i].x, gg.shots[i].y + SHOT_LEN, gg.shots[i].x, gg.shots[i].y);
    }
}

static void __attribute__((constructor)) registerLevel(void) {
    addLevel(&level2);
}
