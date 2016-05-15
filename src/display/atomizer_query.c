#include <stdio.h>
#include <string.h>

#include <M451Series.h>
#include <Button.h>
#include <Display.h>

#include "button.h"
#include "globals.h"
#include "materials.h"
#include "settings.h"
#include "variabletimer.h"
#include "atomizer_query.h"

#include "display_helper.h"

#include "mode.h"
#include "images/ohm.h"

uint8_t newReading(uint16_t oldRes, uint8_t oldTemp, uint16_t *newRes, uint8_t *newTemp) {
    uint16_t lowRes = (100 - BRESDIFFPCT) * g.baseRes / 100;
    uint16_t highRes = (100 + BRESDIFFPCT) * g.baseRes / 100;

    if (oldRes == 0 && gv.fireButtonPressed)
        return 1;

    if (oldRes == 0 && g.baseFromUser == USERSET) {
        g.baseFromUser = USERLOCK;

    }

    switch(g.baseFromUser) {
        case AUTORES:
            if (((*newRes < g.baseRes) && (*newRes > 0)) ||
                  g.baseRes == 0 || oldRes == 0) {
                    baseResSet(*newRes);
                    baseTempSet(*newTemp);
                   // screenOn();
                   // screenOff();
            }
            break;
        case USERSET:
            /* Only prompt during attomizer swap */
            break;
        case USERLOCK:
            if (*newRes < lowRes || *newRes > highRes) {
                gv.fireButtonPressed = 0;
                g.newBaseRes = *newRes;
                g.newBaseTemp = *newTemp;
                g.askUser = 1;
            }
            break;
    }
    return 1;
}

void attyPromptFire(uint8_t status, uint32_t held) {

}

void attyPromptLeft(uint8_t status, uint32_t held) {
    baseFromUserSet(USERSET);
    g.askUser = 0;
}

void attyPromptRight(uint8_t status, uint32_t held) {
    baseResSet(g.newBaseRes);
    baseTempSet(g.newBaseTemp);
    g.askUser = 0;
}

struct buttonHandler attyPromptHandler = {
    .name = "attyPrompt",
    .flags = 0,
    .fire_handler = &attyPromptFire,
    .left_handler = &attyPromptLeft,
    .right_handler = &attyPromptRight,
};

void drawPrompt() {

    if (Display_IsFlipped()) {
        Display_Flip();
    }

    Display_Clear();
    Display_PutText(0, 0,  "Atomizer", FONT_SMALL);
    Display_PutText(0, 10, "Changed", FONT_SMALL);

    Display_PutText(0, 30, "Old", FONT_SMALL);
    buildRow(40, ohm, getFloating, g.baseRes); // resistance

    Display_PutText(0, 70, "New", FONT_SMALL);
    buildRow(80, ohm, getFloating, g.newBaseRes); // resistance

    Display_PutText(0, 115, "Old", FONT_SMALL);
    Display_PutText(40, 115, "New", FONT_SMALL);

    Display_Update();
}

void askUserAboutTheAttomizer() {
    switchHandler(&attyPromptHandler);
    do {;} while (Button_GetState());
    while (g.askUser) {
        if (gv.buttonEvent) {
            handleButtonEvents();
            gv.buttonEvent = 0;
        }
        drawPrompt();
    }
    do {;} while (Button_GetState());
    g.newBaseRes = 0;
    g.newBaseTemp = 0;
    returnHandler();
}
