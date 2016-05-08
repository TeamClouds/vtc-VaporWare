#include <stdint.h>
#include <stdio.h>

#include <Display.h>
#include <USB_VirtualCOM.h>

#include "button.h"
#include "debug.h"
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
    int32_t ItemValues[MAXMENUITEMS];
    uint8_t ItemLoaded[MAXMENUITEMS];
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

void toggleSelect();

void editLeft(uint8_t state, uint32_t duration);
void editRight(uint8_t state, uint32_t duration);
void editSelect(uint8_t state, uint32_t duration);
void doEditEdit(struct menuItem *MI);

struct buttonHandler editButtonHandler = {
    .name = "menuButtonHandler",
    .flags = LEFT_HOLD_EVENT | RIGHT_HOLD_EVENT,
    .fire_handler = &editSelect,
    .left_handler = &editLeft,
    .leftUpdateInterval = 10,
    .right_handler = &editRight,
    .rightUpdateInterval = 10,
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
            MI->selectCallback(mg->ItemValues[mIndex]);
            returnHandler();
            break;
        case TOGGLE:
            toggleSelect();
            MI->toggleCallback(mg->ItemValues[mIndex]);
            break;
        case EDIT:
            switchHandler(&editButtonHandler);
            doEditEdit(MI);
            MI->editCallback(mg->ItemValues[mIndex]);
            returnHandler();
            break;
        case EXITMENU:
            mg->menuOpen = 0;
            break;
        case SUBMENU:
            if (MI->getMenuDef != NULL)
                MI->getMenuDef(MI);
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

skip:
    if (state == BUTTON_PRESS) {
        if (mg->ItemValues[mIndex] - 1 < 0) {
            mg->ItemValues[mIndex] = *MI->count - 1;
        }  else {
            mg->ItemValues[mIndex]--;
        }
    }
    if (MI->getValueCallback(mg->ItemValues[mIndex])[0] == '\0')
        goto skip;
}

void selectRight(uint8_t state, uint32_t duration) {
    uint8_t mIndex = mg->selectIndexToMD[mg->selectIndex];
    struct menuItem *menuItems = *(mg->MD->menuItems);
    struct menuItem *MI = &menuItems[mIndex];
    
skip:
    if (state == BUTTON_PRESS) {
        mg->ItemValues[mIndex]++;
        if (mg->ItemValues[mIndex] >= *MI->count) {
            mg->ItemValues[mIndex] = 0;
        } 
    }
    if (MI->getValueCallback(mg->ItemValues[mIndex])[0] == '\0')
        goto skip;
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

void toggleSelect() {
    uint8_t mIndex = mg->selectIndexToMD[mg->selectIndex];

    mg->ItemValues[mIndex] = !mg->ItemValues[mIndex];
}

void editLeft(uint8_t state, uint32_t duration) {
    uint8_t mIndex = mg->selectIndexToMD[mg->selectIndex];
    struct menuItem *menuItems = *(mg->MD->menuItems);
    struct menuItem *MI = &menuItems[mIndex];
    
    if (state == BUTTON_PRESS ||
        ((duration > 30) && state & BUTTON_HELD)) {
        mg->ItemValues[mIndex] -= MI->editStep;
        if (mg->ItemValues[mIndex] < MI->editMin) {
            mg->ItemValues[mIndex] = MI->editMin;
        } 
    }
}

void editRight(uint8_t state, uint32_t duration) {
    uint8_t mIndex = mg->selectIndexToMD[mg->selectIndex];
    struct menuItem *menuItems = *(mg->MD->menuItems);
    struct menuItem *MI = &menuItems[mIndex];
    
    if (state == BUTTON_PRESS ||
        ((duration > 30) && state & BUTTON_HELD)) {
        mg->ItemValues[mIndex] += MI->editStep;
        if (mg->ItemValues[mIndex] > MI->editMax) {
            mg->ItemValues[mIndex] = MI->editMax;
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

int8_t getItemHeight(struct menuItem *MI, const Font_Info_t *font) {
    uint8_t rowHeight = font->height;
    int used = 0;
    if (MI->type != LINE &&
        MI->type != STARTBOTTOM &&
        MI->type != SPACE &&
        MI->type != END)
        used += rowHeight;

    switch(MI->type) {
        case SELECT:
        case TOGGLE:
        case EDIT:
            used += rowHeight;
            break;
        case LINE:
            used += 1;
            break;
        case SPACE:
            used += MI->rows;
            break;

        default:
            break;
    }
    return used;
}

void drawMenuItem(uint8_t index, uint8_t y, uint8_t x, uint8_t x2, const Font_Info_t *font) {
    struct menuItem *MI = &((*(mg->MD->menuItems))[index]);
    char buff[63];
    char *label;
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
            label = MI->getValueCallback(mg->ItemValues[index]);
            Display_PutText(x2, y + used, label, font);
            break;
        case TOGGLE:
            if (mg->ItemValues[index]) {
                Display_PutText(x2, y + used, MI->on, font);
            } else {
                Display_PutText(x2, y + used, MI->off, font);
            }
            break;
        case EDIT:
            MI->editFormat(mg->ItemValues[index], buff);
            Display_PutText(x2, y + used, buff, font);
            break;
        case LINE:
            Display_PutLine(0, y + used, 63, y + used);
            break;
        case SPACE:
            break;

        default:
            break;
    }
}

void refreshMenu() {
    struct menuItem *MI;
    int i = 0;
    
    while ((MI = &((*(mg->MD->menuItems))[i]))->type != END)
    {
        switch (MI->type) {
            case SELECT:
                mg->ItemValues[i] = MI->getDefaultCallback();
                break;
            case TOGGLE:
                mg->ItemValues[i] = !!*MI->isSet;
                break;
            case EDIT:
                mg->ItemValues[i] = MI->getEditStart();
                break;
            default:
                mg->ItemValues[i] = 0;
                break;    
        }
        i++;
    }
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
        if (mg->ItemLoaded[menuIndex] == 0) {
            switch (MI->type) {
                case SELECT:
                    mg->ItemValues[menuIndex] = MI->getDefaultCallback();
                    break;
                case TOGGLE:
                    mg->ItemValues[menuIndex] = !!*MI->isSet;
                    break;
                case EDIT:
                    mg->ItemValues[menuIndex] = MI->getEditStart();
                    break;
                default:
                    mg->ItemValues[menuIndex] = 0;
                    break;    
            }
            mg->ItemLoaded[menuIndex] = 1;
        }

        if (menuIndex == mg->selectIndexToMD[mg->selectIndex] && mg->editOpen)
            valOffset = 0;
        else
            valOffset = 5;

        if (!findEnd && (MI->hidden == NULL || !MI->hidden())) {
            if (MI->type != STARTBOTTOM) {
                drawMenuItem(menuIndex, rowStart, colStart, colStart + valOffset, mg->MD->font);
            } else {
                findEnd = 1;
            }
        }
        if (MI->hidden == NULL || !MI->hidden()) {
            if (MI->type != LINE &&
                MI->type != STARTBOTTOM &&
                MI->type != SPACE &&
                MI->type != END) {
                mg->selectIndexToMD[mg->menuItemCount] = menuIndex;
                mg->ItemOffsets[mg->menuItemCount] = rowStart;
                mg->menuItemCount++;
            }
            rowStart += getItemHeight(MI, mg->MD->font);
        }

        menuIndex++;
    }
    menuIndex--; // Eat the end
    if (findEnd) {
        uint8_t negFind = 0;
        rowStart = 128; // Bottom of screen
        while ((MI = &menuItems[menuIndex])->type != STARTBOTTOM) {
            
            if (menuIndex == mg->selectIndexToMD[mg->selectIndex] && mg->editOpen)
                valOffset = 0;
            else
                valOffset = 5;

            if (MI->hidden == NULL || !MI->hidden()) {
                rowStart -= getItemHeight(MI, mg->MD->font);
                drawMenuItem(menuIndex, rowStart, colStart, colStart + valOffset, mg->MD->font);

                if (MI->type != LINE &&
                    MI->type != STARTBOTTOM &&
                    MI->type != SPACE &&
                    MI->type != END) {
                    mg->ItemOffsets[mg->menuItemCount - negFind - 1] = rowStart;
                    negFind++;
                }
            }
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
    Display_SetOn(1);
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
