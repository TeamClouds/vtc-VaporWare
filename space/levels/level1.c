#include <Display.h>
#include <variabletimer.h>

#include "game_globals.h"
#include "alien.h"
#include "space_res.h"
#include "level.h"

#define ALIEN_DESTRUCTION_DURATION 1000
#define ALIEN_STEP_INTERVAL 500
#define ALIEN_COUNT 9
#define ALIENS_PER_ROW 3

alien aliens[ALIENS_PER_ROW][ALIENS_PER_ROW];

void init1();

void progress1();

void draw1();

struct level level1 = {
        .index = 1,
        .controlType = LEVEL1,
        .numAliens = ALIEN_COUNT,
        .aliveAliens = ALIEN_COUNT,
        .init = &init1,
        .progress = &progress1,
        .draw = &draw1
};

void init1() {
    uint8_t x = 0, y = 0, spacing = 8;

    // setup aliens
    for (int i = 0; i < ALIENS_PER_ROW; i++) {
        for (int j = 0; j < ALIENS_PER_ROW; j++) {
            aliens[i][j].x = x;
            aliens[i][j].y = y;
            aliens[i][j].w = Bitmap_alien1_width;
            aliens[i][j].h = Bitmap_alien1_height;
            aliens[i][j].health = 1;
            aliens[i][j].lastHitTime = 0;
            aliens[i][j].moveAnimations[0] = Bitmap_alien1;
            aliens[i][j].moveAnimations[1] = Bitmap_alien1_step;

            x += Bitmap_alien1_width;

            // can we fit another on this row?
            if (x + Bitmap_alien1_width + spacing <= DISPLAY_WIDTH) {
                x += spacing;
            } else {
                x = 0;
                y += Bitmap_alien1_height + spacing;
            }
        }
    }
    gg.alienDirection = 1;
    gg.lastAlienShiftTime = 0;
}

uint8_t alienShots = 0;

void progress1() {
    uint32_t fi = uptime % ALIENS_PER_ROW;
    uint32_t fj = uptime % ALIENS_PER_ROW;
    // register shots
    for (int i = ALIENS_PER_ROW - 1; i >= 0; i--) {
        for (int j = 0; j < ALIENS_PER_ROW; j++) {
            if (aliens[i][j].health > 0 && isAlienShot((&aliens[i][j]))) {
                aliens[i][j].health = 0;
                aliens[i][j].lastHitTime = uptime;
                level1.aliveAliens--;
                gg.userScore++;
            }
            if (isAlienFiring(&aliens[i][j])) {
                if (progressShot(&aliens[i][j].shots[0]) == SHOT_OUT_OF_BOUNDS) {
                    alienShots--;
                } else if (isShipShot(&aliens[i][j].shots[0], gg.shipCoords[0], gg.shipCoords[1],
                                      Bitmap_ship_width, Bitmap_ship_height)) {
                    alienShots--;
                    gg.playerHealth--;
                    clearShot(&aliens[i][j].shots[0]);
                }
            } else if (aliens[i][j].health > 0 && fi == i && fj == j && alienShots < 1) {
                alienShots++;
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
            gg.xEnemyOffset++;
            if (gg.xEnemyOffset == 10) {
                gg.alienDirection = 0;
            }
        } else {
            gg.xEnemyOffset--;
            if (gg.xEnemyOffset == 0) {
                gg.alienDirection = 1;
            }
        }
        gg.lastAlienShiftTime = uptime;
    }
}

void draw1() {
    // render aliens
    for (int i = 0; i < ALIENS_PER_ROW; i++) {
        for (int j = 0; j < ALIENS_PER_ROW; j++) {
            if (aliens[i][j].health > 0) {
                // two-step
                Display_PutPixels(aliens[i][j].x + gg.xEnemyOffset,
                                  aliens[i][j].y + gg.yEnemyOffset,
                                  aliens[i][j].moveAnimations[gg.xEnemyOffset % 2 == 0 ? 1 : 0],
                                  aliens[i][j].w,
                                  aliens[i][j].h);
            } else if (aliens[i][j].health == 0 &&
                       aliens[i][j].lastHitTime + ALIEN_DESTRUCTION_DURATION > uptime) {
                // explosion animation
                Display_PutPixels(aliens[i][j].x + gg.xEnemyOffset,
                                  aliens[i][j].y + gg.yEnemyOffset,
                                  Bitmap_alien1_shot,
                                  Bitmap_alien1_shot_width,
                                  Bitmap_alien1_shot_height);
            }

            if (isAlienFiring(&aliens[i][j])) {
                Display_PutLine(aliens[i][j].shots[0].x,
                                aliens[i][j].shots[0].y,
                                aliens[i][j].shots[0].x,
                                aliens[i][j].shots[0].y + SHOT_LEN);
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
    addLevel(&level1);
}
