#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "globals.h"
#include "display_helper.h"

/* TODO: Add margins/padding/absolute/single element overrides */
struct layoutProperties {
    int8_t X;
    int8_t Y;

    int8_t W; // Does not include margins
    int8_t H;

    int8_t mXl;
    int8_t mXr;
    int8_t mYt;
    int8_t mYb;

    int8_t pXl;
    int8_t pXr;
    int8_t pYt;
    int8_t pYb;

    uint8_t flags;
};

struct textPriv {
    char *text;
    uint8_t len;
    const Font_Info_t *font;

    void (*const getText)(char *text, uint8_t len);
    void *priv;

};

uint8_t textGetDimensions(void *priv, uint8_t *args, struct layoutProperties *layout);
uint8_t textDraw(void *priv, uint8_t *args, struct layoutProperties *layout);


struct displayItem {
    char label[9];
    uint8_t args;
    uint8_t (*const getDimensions)(void *priv, uint8_t *args, struct layoutProperties *layout);
    uint8_t (*const drawAt)(void *priv, uint8_t *args, struct layoutProperties *layout);
    void *priv;
};

void getTextDimensions(const Font_Info_t *font, char *text, struct layoutProperties *layout);

enum {
    TOP = 0,
    VCENTER = 1,
    BOTTOM = 2,
    LEFT = 0,
    CENTER = 4,
    RIGHT = 8,
    ABSOLUTE = 16,
    TOPDOWN = 0,
    LEFTTORIGHT = 32
};

uint8_t drawScreen(uint8_t *items);
uint8_t drawItem(const struct displayItem *DI, uint8_t *args, struct layoutProperties *layout);

void setupScreen();
void updateScreen();
void fadeInTransition();
void displayCharging();
void getString(char *buff, uint8_t len, char *state);
void showMenu();

uint8_t (* customGetDimensions)(struct layoutProperties *layout);
uint8_t (* customDrawAt)(struct layoutProperties *layout);

#endif
