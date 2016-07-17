#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Display.h"
#include "TimerUtils.h"

#include "button.h"
#include "display.h"
#include "drawables.h"
#include "variabletimer.h"
#include "debug.h"

uint8_t block[] = { 0x0F, 0x0F, 0x0F, 0x0F};
// 0000
// 0110
// 0110
// 0000
uint8_t SQBitmap0[] = {0x0, 0x6, 0x6, 0x0, 0};
// SQBitmap3,2,1 = SQBitmap 0

// 0100 0000 1100 0010
// 0100 1110 0100 1110
// 0110 1000 0100 0000
uint8_t LBitmap0[] = {0x4, 0x4, 0x6, 0};
uint8_t LBitmap1[] = {0x0, 0xE, 0x8, 0};
uint8_t LBitmap2[] = {0xC, 0x4, 0x4, 0};
uint8_t LBitmap3[] = {0x2, 0xE, 0x0, 0};

// 0100 1000 0110 0000
// 0100 1110 0100 1110
// 1100 0000 0100 0010
uint8_t JBitmap0[] = {0x4, 0x4, 0xC, 0};
uint8_t JBitmap1[] = {0x8, 0xE, 0x0, 0};
uint8_t JBitmap2[] = {0x6, 0x4, 0x4, 0};
uint8_t JBitmap3[] = {0x0, 0xE, 0x2, 0};

// 0100 0000
// 0110 0110
// 0010 1100
uint8_t SBitmap0[] = {0x4, 0x6, 0x2, 0};
uint8_t SBitmap1[] = {0x0, 0x6, 0xC, 0};
// SBitmap2 = SBitmap0, SBitmap3 = SBitmap1

// 0010 0000
// 0110 1100
// 0100 0110
uint8_t ZBitmap0[] = {0x2, 0x6, 0x4, 0};
uint8_t ZBitmap1[] = {0x0, 0xC, 0x6, 0};
// ZBitmap2 = ZBitmap0, ZBitmap3 = ZBitmap1

// 0100 0000 0100 0100
// 0110 1110 1100 1110
// 0100 0100 0100 0000
uint8_t TBitmap0[] = {0x4, 0x6, 0x4, 0};
uint8_t TBitmap1[] = {0x0, 0xE, 0x4, 0};
uint8_t TBitmap2[] = {0x4, 0xC, 0x4, 0};
uint8_t TBitmap3[] = {0x4, 0xE, 0x0, 0};

// 0010 0000
// 0010 0000
// 0010 1111
// 0010 0000
uint8_t IBitmap0[] = {0x2, 0x2, 0x2, 0x2, 0};
uint8_t IBitmap1[] = {0x0, 0x0, 0xF, 0x0, 0};
// IBitmap2 = IBitmap0, IBitmap 3 = IBitmap1

enum {
    SQUARE,
    LPIECE,
    JPIECE,
    SPIECE,
    ZPIECE,
    TPIECE,
    IPIECE,
    NUMPIECES
};

uint8_t widths[NUMPIECES] = {
    [SQUARE] = 4,
    [LPIECE] = 4,
    [JPIECE] = 4,
    [SPIECE] = 4,
    [ZPIECE] = 4,
    [TPIECE] = 4,
    [IPIECE] = 4
};

uint8_t heights[NUMPIECES] = {
    [SQUARE] = 4,
    [LPIECE] = 3,
    [JPIECE] = 3,
    [SPIECE] = 3,
    [ZPIECE] = 3,
    [TPIECE] = 3,
    [IPIECE] = 4
};

uint8_t *pieces[][4] = {
    [SQUARE] = {SQBitmap0, SQBitmap0, SQBitmap0, SQBitmap0},
    [LPIECE] = {LBitmap0, LBitmap1, LBitmap2, LBitmap3},
    [JPIECE] = {JBitmap0, JBitmap1, JBitmap2, JBitmap3},
    [SPIECE] = {SBitmap0, SBitmap1, SBitmap0, SBitmap1},
    [ZPIECE] = {ZBitmap0, ZBitmap1, ZBitmap0, ZBitmap1},
    [TPIECE] = {TBitmap0, TBitmap1, TBitmap2, TBitmap3},
    [IPIECE] = {IBitmap0, IBitmap1, IBitmap0, IBitmap1},
};


/* We have (roughly)63x128 to work with

  Width:
    3px - Left Margin
    1px - left border @3
   40px - Playing field (10 2x2 squares) @4
    1px - right border @44
    3px - right margin @45
    1px - preview left border @48
   10px - preview (always draws 'vertical' orientation - no more than 8px used for drawing)
        - Contents are started at 50,1
    1px - preview right border @60
    Total: 6px

  Height:
   16px - 'Masked drawing area' (top 4 rows of playing area so pieces can fall in smoothly)
   80px - playing area
    1px - Bottom playing area
   30px - Score/Level details
    Total: 127px
*/

uint8_t decorations[] = {

    DRAWLINE, ATTRGROUP, ATTRALIGN, ABSOLUTE, ENDATTRGROUP, 1,0,1,88, /* Left boarder */
    DRAWLINE, ATTRGROUP, ATTRALIGN, ABSOLUTE, ENDATTRGROUP, 42,0,42,88, /* Right boarder */
    DRAWLINE, ATTRGROUP, ATTRALIGN, ABSOLUTE, ENDATTRGROUP, 1,88,42,88, /* bottom boarder */
    DRAWLINE, ATTRGROUP, ATTRALIGN, ABSOLUTE, ENDATTRGROUP, 1,8,42,8,
    DRAWLINE, ATTRGROUP, ATTRALIGN, ABSOLUTE, ENDATTRGROUP, 42,0,63,0,
    DRAWLINE, ATTRGROUP, ATTRALIGN, ABSOLUTE, ENDATTRGROUP, 42,20,63,20,
    DRAWLINE, ATTRGROUP, ATTRALIGN, ABSOLUTE, ENDATTRGROUP, 63,0,63,20,
/*
    DRAWLINE, ATTRGROUP, ATTRALIGN, ABSOLUTE, ENDATTRGROUP, 6,1, 42,87,
    DRAWLINE, ATTRGROUP, ATTRALIGN, ABSOLUTE, ENDATTRGROUP, 6,87,42,1,
    */
    VAPTRISFIELD, ATTRGROUP, ATTRLOCATION, 2, 0, ENDATTRGROUP,
    VAPTRISSCORE, ATTRGROUP, ATTRLOCATION, 2, 90, ENDATTRGROUP,
    VAPTRISLEVEL, ATTRGROUP, ATTRLOCATION, 2, 100, ENDATTRGROUP,
    VAPTRISNEXT, ATTRGROUP, ATTRLOCATION, 44, 2, ENDATTRGROUP,
    VAPTRISPIECE,
    LD
};


// 1 00000 00000 1 0000
#define EMPTYROW 0x8010
// 1 11111 11111 10000
#define COMPLETEROW 0xFFF0

// TODO: make it heap.
// 12 bits are used
uint16_t playingField[23] = {0};


struct gameState {
    uint8_t curPiece;
    uint8_t next[4];
    uint8_t curRot;
    uint8_t curX;
    uint8_t curW;
    uint8_t curY;
    uint8_t curH;
    uint8_t X0;
    uint8_t Y0;
    uint8_t delay;
    uint8_t level;
    uint16_t rows;
    uint32_t score;
    uint8_t ltimer;
    uint8_t quit;
};

struct gameState vaptrisState = { 0 };

void setVapeSpeed() {
    vaptrisState.delay = 5 * (11 - vaptrisState.level + 1) * 10;
}

void setVapePiece(uint8_t piece) {
    vaptrisState.curPiece = piece;
    vaptrisState.curW = widths[vaptrisState.curPiece];
    vaptrisState.curH = heights[vaptrisState.curPiece];
}

void initVaptris() {
    setVapePiece(uptime % NUMPIECES);
    vaptrisState.next[0] = uptime / 10  % NUMPIECES;
    vaptrisState.next[1] = uptime / 100 % NUMPIECES;
    vaptrisState.next[2] = uptime / 200 % NUMPIECES;
    vaptrisState.next[3] = uptime / 300 % NUMPIECES;
    vaptrisState.curRot = 0;
    vaptrisState.X0 = 0;
    vaptrisState.Y0 = 0;
    vaptrisState.rows = 0;
    vaptrisState.level = 0;
    vaptrisState.score = 0;
    vaptrisState.quit = 0;
    setVapeSpeed();
}

void getNextPiece() {
    setVapePiece(vaptrisState.next[0]);
    memmove(&vaptrisState.next[0], &vaptrisState.next[1], 3);
    vaptrisState.next[3] = uptime % NUMPIECES;
    vaptrisState.curRot = 0;
    setVapeSpeed();
    vaptrisState.curY = 0;
    vaptrisState.curX = 5 - vaptrisState.curW / 2; // Todo, make this a lookup
}

void clearRows() {
    uint8_t clearCount = 0;
    for(int i = 0; i < 22; i++) {
        if (playingField[i] == COMPLETEROW) {
            memmove(&playingField[1], &playingField[0], i * sizeof(uint16_t));
            playingField[0] = EMPTYROW;
            clearCount++;
            vaptrisState.rows++;
            if (vaptrisState.rows % 10 == 0)
                vaptrisState.level++;
        }
    }
    switch (clearCount) {
        case 1:
            vaptrisState.score += 40 * (vaptrisState.level + 1);
            break;
        case 2:
            vaptrisState.score += 100 * (vaptrisState.level + 1);
            break;
        case 3:
            vaptrisState.score += 300 * (vaptrisState.level + 1);
            break;
        case 4:
            vaptrisState.score += 1200 * (vaptrisState.level + 1);
            break;
        default:
            break;
    }
}

uint8_t isBoardClear() {
    for(int i = 0; i < 22; i++) {
        if (playingField[i] != EMPTYROW)
            return 0;
    }

    return 1;
}

int checkColission() {
    uint8_t *curPc = pieces[vaptrisState.curPiece][vaptrisState.curRot];
    uint16_t mask = 0;
    uint8_t row = 0;
    for ( row = 0; row < heights[vaptrisState.curPiece]; row++ ) {
        mask = curPc[row];
        uint8_t index = (vaptrisState.curY - vaptrisState.Y0)/4 + row;
        uint8_t shift = 12 - (vaptrisState.curX) - 1;

        if (playingField[index + 1] & (mask << shift))
            return 1;
    }
    return 0;
}

void vaptrisMoveLeft(uint8_t status, uint32_t held) {
    if ((status & (BUTTON_PRESS | BUTTON_HELD))) {
        vaptrisState.curX--;
        if (checkColission())
            vaptrisState.curX++;
    }
}

void vaptrisMoveRight(uint8_t status, uint32_t held) {
    if ((status & (BUTTON_PRESS))) {
        vaptrisState.curX++;
        if (checkColission())
            vaptrisState.curX--;
    }
}

void vaptrisRotate(uint8_t status, uint32_t held) {
    if (!status) {
        vaptrisState.curRot++;
        vaptrisState.curRot %= 4;
        if (checkColission()) {
            vaptrisState.curRot--;
            vaptrisState.curRot %= 4;
        }
        vaptrisState.curW = widths[vaptrisState.curPiece];
        vaptrisState.curH = heights[vaptrisState.curPiece];
    }
    if (status & BUTTON_HELD && held > 500) {
        // Drop
    }
    if (status & BUTTON_HELD && held > 3000) {
        vaptrisState.quit = 1;
    }
}


struct buttonHandler vaptrisHandler = {
        .name = "mainButtons",
        .flags = FIRE_HOLD_EVENT,

        .fire_handler = &vaptrisRotate,
        .fireUpdateInterval = 150,

        .left_handler = &vaptrisMoveLeft,
        .leftUpdateInterval = 150,

        .right_handler = &vaptrisMoveRight,
        .rightUpdateInterval = 150,

};

void runvaptris() {
    switchHandler(&vaptrisHandler);
    if (!vaptrisState.ltimer)
         vaptrisState.ltimer = requestTimerSlot();

    requestTimer(vaptrisState.ltimer, TimerHighRes);
    waitForFasterTimer(TimerHighRes);
    initVaptris();
    getNextPiece();

    for (int clear = 0; clear < 22; clear++)
        playingField[clear] = EMPTYROW;

    playingField[22] = 0xFFFF;

    uint32_t nextstep = uptime + vaptrisState.delay;

    while (1) {
        setupScreen();

        if (gv.buttonEvent) {
            handleButtonEvents();
            gv.buttonEvent = 0;
        }

        if (vaptrisState.quit)
            goto gameover;

        if (uptime > nextstep) {

            nextstep = uptime + vaptrisState.delay;

            vaptrisState.curY++;

            if (playingField[2] != EMPTYROW)
                goto gameover;

            if ((vaptrisState.curY % 4 == 3) ) {

                uint8_t *curPc = pieces[vaptrisState.curPiece][vaptrisState.curRot];
                uint16_t mask = 0;
                uint8_t row = 0;

                for ( row = 0; row < heights[vaptrisState.curPiece]; row++ ) {

                    uint8_t index = (vaptrisState.curY - vaptrisState.Y0)/4;
                    uint8_t shift = 12 - (vaptrisState.curX) - 1;

                    mask = curPc[row] << shift;

                    if (playingField[index + row + 1] & mask) {
                        for (int i = 0; i < vaptrisState.curH; i++)
                            playingField[index + i] |= curPc[i] << shift;

                        vaptrisState.score += 10 * (vaptrisState.level + 1);

                        clearRows();
                        if (isBoardClear())
                            vaptrisState.score += 2000 * (vaptrisState.level + 1);

                        getNextPiece();
                    }
                    if (vaptrisState.curY > 90)
                        goto gameover;
                }
            }
        }
        drawScreen(decorations);
        Display_Update();
    }

gameover:
    requestTimer(vaptrisState.ltimer, TimerIdle);
    vaptrisState.curY = 0;
    returnHandler();
}

uint8_t vaptrisGetDimensions(void *priv, uint8_t *args, struct layoutProperties *object) {
    object->W = 40;
    object->H = 88;
    return 0;
}

uint8_t vaptrisDraw(void *priv, uint8_t *args, struct layoutProperties *object) {
    int i, j;
    int8_t x = 0;
    int8_t y = 0;
    vaptrisState.X0 = object->X;
    vaptrisState.Y0 = object->Y;
    for(i = 0; i < 22; i++) {
        /* if (playingField[i] != EMPTYROW) */ {
            for (j = 1; j <= 10; j++) {
                if (playingField[i] & (1 << (15 - j))) {
                    x = object->X + 4 * (j - 1);
                    y = object->Y + 4 * i;

                    Display_PutPixels(x, y, block,4,4);
                }
            }
        }
    }
    return 0;
}

const struct displayItem __vaptris = {
    .label = "vaptris",
    .getDimensions = &vaptrisGetDimensions,
    .drawAt = &vaptrisDraw,
    .args = 0
};

uint8_t vaptrisPieceGetDimensions(void *priv, uint8_t *args, struct layoutProperties *object) {
    object->W = 16;
    object->H = 16;
    return 0;
}

void drawPiece(uint8_t piece, uint8_t rot, int X, int Y) {
    int i = 0, j;
    int8_t x = 0;
    int8_t y = 0;
    uint8_t row = 0;
    uint8_t mask;
    uint8_t *curPc = pieces[piece][rot];
    for ( row = 0; row < heights[piece]; row++ ) {
        mask = curPc[row];
        for (j = 0; j < widths[piece]; j++) {
            if (mask & (1 << (3 - j))) {
                x = X + 4 * j;
                y = Y + 4 * row;
                Display_PutPixels(x, y, block,4,4);
            }
        }
        i++;
    }
}

uint8_t vaptrisPieceDraw(void *priv, uint8_t *args, struct layoutProperties *object) {
    drawPiece(vaptrisState.curPiece, vaptrisState.curRot,
              vaptrisState.X0 + 4 * vaptrisState.curX,
              vaptrisState.Y0 + vaptrisState.curY);
    return 0;
}

const struct displayItem __vaptrisPiece = {
    .label = "vaptris",
    .getDimensions = &vaptrisPieceGetDimensions,
    .drawAt = &vaptrisPieceDraw,
    .args = 0
};

void vaptrisScoreText(char *text, uint8_t len) {
    // S: 4294967296\0
    sniprintf(text, len, "S: %ld", vaptrisState.score);
}

struct textPriv vaptrisScore = {
    .font = FONT_SMALL,
    .len = 14,
    .getText = &vaptrisScoreText
};

const struct displayItem vaptrisScoreTextS = {
    .label = "vapscr",
    .getDimensions = &textGetDimensions,
    .drawAt = &textDraw,
    .priv = &vaptrisScore
};

void vaptrisLevelText(char *text, uint8_t len) {
    // L: 6553.9\0
    sniprintf(text, len, "L: %d.%d", vaptrisState.level, vaptrisState.rows % 10);
}

struct textPriv vaptrisLevel = {
    .font = FONT_SMALL,
    .len = 10,
    .getText = &vaptrisLevelText
};

const struct displayItem vaptrisLevelTextS = {
    .label = "atyTextL",
    .getDimensions = &textGetDimensions,
    .drawAt = &textDraw,
    .priv = &vaptrisLevel
};

uint8_t vaptrisNPieceGetDimensions(void *priv, uint8_t *args, struct layoutProperties *object) {
    object->W = 16;
    object->H = 16;
    return 0;
}

uint8_t vaptrisNPieceDraw(void *priv, uint8_t *args, struct layoutProperties *object) {
    drawPiece(vaptrisState.next[0], 0, object->X, object->Y);
    return 0;
}

const struct displayItem __vaptrisNPiece = {
    .label = "vaptris",
    .getDimensions = &vaptrisNPieceGetDimensions,
    .drawAt = &vaptrisNPieceDraw,
    .args = 0
};

static void __attribute__((constructor)) registerVaptrisDrawables(void) {
    drawables[VAPTRISFIELD] = &__vaptris;
    drawables[VAPTRISPIECE] = &__vaptrisPiece;
    drawables[VAPTRISSCORE] = &vaptrisScoreTextS;
    drawables[VAPTRISLEVEL] = &vaptrisLevelTextS;
    drawables[VAPTRISNEXT] = &__vaptrisNPiece;
}