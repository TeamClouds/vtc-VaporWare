#include <stdint.h>
#include <stdio.h>

#include <Display.h>
#include <USB_VirtualCOM.h>

#include "button.h"
#include "globals.h"
#include "menu.h"

enum {
    EDITCLOSED = 0,
    EDITOPEN = 1,
    SELECTOPEN = 2
};

void drawMenu();

struct menuGlobals {
    uint8_t menuOpen;
    uint8_t editOpen;
    uint8_t selectIndex;
    uint8_t menuItemCount;
    uint8_t ItemOffsets[MAXMENUITEMS];
    uint8_t selectIndexToMD[MAXMENUITEMS];
    struct menuDefinition *MD;
};

struct menuGlobals *mg;

void menuLeft(uint8_t state, uint32_t duration);
void menuRight(uint8_t state, uint32_t duration);
void menuSelect(uint8_t state, uint32_t duration);

struct buttonHandler menuButtonHandler = {
    .name = "menuButtonHandler",
    .fire_handler = &menuSelect,
    .left_handler = &menuLeft,
    .right_handler = &menuRight,
};

void selectLeft(uint8_t state, uint32_t duration);
void selectRight(uint8_t state, uint32_t duration);
void selectSelect(uint8_t state, uint32_t duration);
void doSelectEdit(struct menuItem *MI);

struct buttonHandler selectButtonHandler = {
    .name = "selectButtonHandler",
    .fire_handler = &selectSelect,
    .left_handler = &selectLeft,
    .right_handler = &selectRight,
};

void editLeft(uint8_t state, uint32_t duration);
void editRight(uint8_t state, uint32_t duration);
void editSelect(uint8_t state, uint32_t duration);
void doEditEdit(struct menuItem *MI);

struct buttonHandler editButtonHandler = {
    .name = "menuButtonHandler",
    .fire_handler = &editSelect,
    .left_handler = &editLeft,
    .right_handler = &editRight,
};

void menuLeft(uint8_t state, uint32_t duration) {
    
    if (state == BUTTON_REL) {
        if (mg->selectIndex - 1 < 0) {
            mg->selectIndex = mg->menuItemCount - 1;
        } else {
            mg->selectIndex--;
        }
    }
}

void menuRight(uint8_t state, uint32_t duration) {
    
    if (state == BUTTON_REL) {
        if (mg->selectIndex + 1 > mg->menuItemCount - 1) {
            mg->selectIndex = 0;
        } else {
            mg->selectIndex++;
        }
    }
}

void menuSelect(uint8_t state, uint32_t duration) {
    if (state == BUTTON_REL)
        return;
    
    struct menuGlobals *t;
    uint8_t mIndex = mg->selectIndexToMD[mg->selectIndex];
    struct menuItem *menuItems = *(mg->MD->menuItems);
    struct menuItem *MI = &menuItems[mIndex];
    
    returnHandler();
    switch (MI->type) {
        case ACTION:
            MI->actionCallback();
            break;
        case SELECT:
            switchHandler(&selectButtonHandler);
            doSelectEdit(MI);
            MI->selectCallback(MI->startAt);
            returnHandler();
            break;
        case EDIT:
            switchHandler(&editButtonHandler);
            doEditEdit(MI);
            MI->editCallback(MI->editStart);
            returnHandler();
            break;
        case EXITMENU:
            mg->menuOpen = 0;
            break;
        case SUBMENU:
            t = mg;
            runMenu(MI->subMenu);
            mg = t;
            break;

    }
    switchHandler(&menuButtonHandler);

}


void selectLeft(uint8_t state, uint32_t duration) {
    uint8_t mIndex = mg->selectIndexToMD[mg->selectIndex];
    struct menuItem *menuItems = *(mg->MD->menuItems);
    struct menuItem *MI = &menuItems[mIndex];

    if (state == BUTTON_PRESS) {
        if (MI->startAt - 1 < 0) {
            MI->startAt = MI->count - 1;
        }  else {
            MI->startAt--;
        }
    }
}

void selectRight(uint8_t state, uint32_t duration) {
    uint8_t mIndex = mg->selectIndexToMD[mg->selectIndex];
    struct menuItem *menuItems = *(mg->MD->menuItems);
    struct menuItem *MI = &menuItems[mIndex];
    
    if (state == BUTTON_PRESS) {
        MI->startAt++;
        if (MI->startAt >= MI->count) {
            MI->startAt = 0;
        } 
    }
}

void selectSelect(uint8_t state, uint32_t duration) {
    if (state == BUTTON_PRESS)
        mg->editOpen = EDITCLOSED;
}

void doSelectEdit(struct menuItem *MI) {
    mg->editOpen = SELECTOPEN;
    while (mg->editOpen) {
        if (gv.buttonEvent) {
            handleButtonEvents();
        }
        drawMenu();
    }
}

void editLeft(uint8_t state, uint32_t duration) {
    uint8_t mIndex = mg->selectIndexToMD[mg->selectIndex];
    struct menuItem *menuItems = *(mg->MD->menuItems);
    struct menuItem *MI = &menuItems[mIndex];
    
    if (state == BUTTON_PRESS ||
        state == BUTTON_HELD) {
        MI->editStart -= MI->editStep;
        if (MI->editStart < MI->editMin) {
            mg->selectIndex = MI->editMin;
        } 
    }
}

void editRight(uint8_t state, uint32_t duration) {
    uint8_t mIndex = mg->selectIndexToMD[mg->selectIndex];
    struct menuItem *menuItems = *(mg->MD->menuItems);
    struct menuItem *MI = &menuItems[mIndex];
    
    if (state == BUTTON_PRESS ||
        state == BUTTON_HELD) {
        MI->editStart += MI->editStep;
        if (MI->editStart > MI->editMax) {
            mg->selectIndex = MI->editMax;
        } 
    }
}

void editSelect(uint8_t state, uint32_t duration) {
    if (state == BUTTON_PRESS)
        mg->editOpen = EDITCLOSED;
}

void doEditEdit(struct menuItem *MI){
    mg->editOpen = EDITOPEN;
    while (mg->editOpen) {
        if (gv.buttonEvent) {
            handleButtonEvents();
        }
        drawMenu();
    }
}

int drawMenuItem(struct menuItem *MI, uint8_t y, uint8_t x, uint8_t x2, const Font_Info_t *font) {
    char buff[63];
    uint8_t rowHeight = font->height;
    

    int used = 0;
    if (MI->type != LINE &&
        MI->type != STARTBOTTOM &&
        MI->type != SPACE &&
        MI->type != END) {
        
        Display_PutText(x, y + used, MI->label, font);
        used += rowHeight;
    }
    switch(MI->type) {
        case SELECT:
            Display_PutText(x2, y + used, (*MI->items)[MI->startAt], font);
            used += rowHeight;
            break;
        case EDIT:
            MI->editFormat(MI->editStart, buff);
            Display_PutText(x2, y + used, buff, font);
            used += rowHeight;
            break;
        case LINE:
            Display_PutLine(0, y + used, 63, y + used);
            used += rowHeight;
            break;
        case SPACE:
            used += MI->rows;
            break;

        default:
            break;
    }
    return used;
}

void drawMenu() {
    uint8_t menuIndex = 0;
    uint8_t rowStart = 0, colStart = 10, valOffset = 5;
    mg->menuItemCount = 0;
    uint8_t rowHeight = mg->MD->font->height;

    uint8_t findEnd = 0;

    struct menuItem *menuItems = *(mg->MD->menuItems);
    struct menuItem *MI;

    Display_Clear();

    while ((MI = &menuItems[menuIndex])->type != END) {
        if (menuIndex == mg->selectIndex && mg->editOpen)
            valOffset = 0;
        else
            valOffset = 5;

        if (!findEnd) {
            if (MI->type != STARTBOTTOM) {
                mg->ItemOffsets[mg->menuItemCount] = rowStart;
                rowStart += drawMenuItem(MI, rowStart, colStart, colStart + valOffset, mg->MD->font);
            } else {
                findEnd = 1;
            }
        }
        if (MI->type != LINE &&
            MI->type != STARTBOTTOM &&
            MI->type != SPACE &&
            MI->type != END) {
            mg->selectIndexToMD[mg->menuItemCount] = menuIndex;
            mg->menuItemCount++;
        }
        
        menuIndex++;
    }

    if (findEnd) {
        uint8_t negFind = 0;
        rowStart = 120 - rowHeight;
        while ((MI = &menuItems[menuIndex])->type != STARTBOTTOM) {
            if (menuIndex == mg->selectIndex && mg->editOpen)
                valOffset = 0;
            else
                valOffset = 5;

            if (MI->type != LINE &&
                MI->type != STARTBOTTOM &&
                MI->type != SPACE &&
                MI->type != END) {
                negFind++;
                mg->selectIndexToMD[mg->menuItemCount - negFind] = menuIndex;
            }

            mg->ItemOffsets[mg->menuItemCount - negFind] = rowStart;
            rowStart -= drawMenuItem(MI, rowStart, colStart, colStart + valOffset, mg->MD->font);
            menuIndex--;
        }
    }

    switch (mg->editOpen) {
        case EDITCLOSED:
            Display_PutText(0, mg->ItemOffsets[mg->selectIndex], mg->MD->cursor, mg->MD->font);
            break;
        case EDITOPEN:
            Display_PutText(0, mg->ItemOffsets[mg->selectIndex] + rowHeight, mg->MD->less_sel, mg->MD->font);
            Display_PutText(50, mg->ItemOffsets[mg->selectIndex] + rowHeight, mg->MD->more_sel, mg->MD->font);
            break;
        case SELECTOPEN:
            Display_PutText(0, mg->ItemOffsets[mg->selectIndex] + rowHeight, mg->MD->prev_sel, mg->MD->font);
            Display_PutText(50, mg->ItemOffsets[mg->selectIndex] + rowHeight, mg->MD->next_sel, mg->MD->font);
            break;
    }
    Display_Update();
}

void runMenu(struct menuDefinition *menuDef) {
    //while (USB_VirtualCOM_GetAvailableSize() == 0){;}
    struct menuGlobals _mg = {0};
    mg = &_mg;

    mg->selectIndex = 0;
    mg->menuOpen = 1;
    mg->MD = menuDef;

    switchHandler(&menuButtonHandler);
    
    while (mg->menuOpen) {
        if (gv.buttonEvent) {
            handleButtonEvents();
        }
        drawMenu();
    }

    returnHandler();
}