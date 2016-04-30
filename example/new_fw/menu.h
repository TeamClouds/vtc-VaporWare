#ifndef __MENU_H
#define __MENU_H
#include <stdint.h>
#include <Font.h>

enum {
    ACTION,
    SELECT,
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

    /* ACTION */
    void (*actionCallback)(void);

    /* SELECT */
    char *(*items)[];
    uint16_t startAt; // Might be modified
    uint8_t count;
    void (*selectCallback)(uint16_t index);

    /* EDIT */
    int32_t editMin;
    int32_t editMax;
    int32_t editStart; // Might be modified
    int32_t editStep;
    void (*editFormat)(int32_t value, char *formatted);
    void (*editCallback)(int32_t);

    /* SPACE */
    uint8_t rows;

    /* SUBMENU */
    struct menuDefinition *subMenu;
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

#endif