#ifndef __MENU_H
#define __MENU_H
#include <stdint.h>
#include "font/font_vaporware.h"

enum {
    ACTION,
    SELECT,
    TOGGLE,
    EDIT,
    LINE,
    SPACE,
    SUBMENU,
    STARTBOTTOM,
    EXITMENU,
    END
};

struct menuDefinition;

struct menuItem {
    const uint8_t type;
    const char *label;
    int (*const hidden)(void);

    /* ACTION */
    void (*const actionCallback)(void);

    /* SELECT */
    const char *(*const items)[];
    const uint8_t *const count;
    uint8_t (*const getDefaultCallback)();
    char *(*const getValueCallback)(uint8_t index);
    void (*const selectCallback)(uint16_t index);

    /* TOGGLE */
    const uint8_t *isSet;
    const char on[5];
    const char off[5];
    void (*const toggleCallback)(uint8_t on);

    /* EDIT */
    const int32_t editMin;
    const int32_t editMax;
    int32_t (*const getEditStart)();
    const int32_t editStep;
    void (*const editFormat)(int32_t value, char *formatted);
    void (*const editCallback)(int32_t);

    /* SPACE */
    const uint8_t rows;

    /* SUBMENU */
    const struct menuDefinition *subMenu;
    const struct menuDefinition *const (*const getMenuDef)(const struct menuItem *this);
};

// Per layer of menu.  Could bump as high as 255, but wastes memory
#define MAXMENUITEMS 32
enum {
    SHOWMENU,
};

struct menuDefinition {
    const char * const name;
    const Font_Info_t *font;
    const uint8_t flags;

    const char cursor[2];
    const char prev_sel[2];
    const char next_sel[2];
    const char less_sel[2];
    const char more_sel[2];

    const struct menuItem (*const menuItems)[];
};


void runSubMenu(const struct menuDefinition *menuDef);
void runMenu(const struct menuDefinition *menuDef);

void refreshMenu();

#endif
