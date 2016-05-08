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
    uint8_t type;
    const char *label;
    int (*hidden)(void);

    /* ACTION */
    void (*actionCallback)(void);

    /* SELECT */
    char *(*items)[];
    uint8_t *count;
    uint8_t (*getDefaultCallback)();
    char *(*getValueCallback)(uint8_t index);
    void (*selectCallback)(uint16_t index);

    /* TOGGLE */
    uint8_t *isSet;
    char on[5];
    char off[5];
    void (*toggleCallback)(uint8_t on);

    /* EDIT */
    int32_t editMin;
    int32_t editMax;
    int32_t (*getEditStart)();
    int32_t editStep;
    void (*editFormat)(int32_t value, char *formatted);
    void (*editCallback)(int32_t);

    /* SPACE */
    uint8_t rows;

    /* SUBMENU */
    struct menuDefinition *subMenu;
    void (*getMenuDef)(struct menuItem *this);
};

// Per layer of menu.  Could bump as high as 255, but wastes memory
#define MAXMENUITEMS 32
enum {
    SHOWMENU,
};

struct menuDefinition {
    const char *name;
    const Font_Info_t *font;
    uint8_t flags;

    const char cursor[2];
    const char prev_sel[2];
    const char next_sel[2];
    const char less_sel[2];
    const char more_sel[2];

    struct menuItem (*menuItems)[];
};



void runMenu(struct menuDefinition *menuDef);
void refreshMenu();

#endif
