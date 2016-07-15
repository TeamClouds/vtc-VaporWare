/*
 * This file is part of eVic SDK.
 *
 * eVic SDK is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * eVic SDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with eVic SDK.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2016 ReservedField
 * Copyright (C) 2016 kfazz
 */

#include <stdio.h>
#include <string.h>

#include <M451Series.h>
#include <Display.h>
#include <Atomizer.h>
#include <TimerUtils.h>

#include <Button.h>

#include "display.h"
#include "drawables.h"
#include "font/font_vaporware.h"
#include "globals.h"
#include "helper.h"
#include "display_helper.h"
#include "settings.h"

#include "variabletimer.h"
#include "debug.h"



const struct displayItem *drawables[LASTDRAWABLE];

// NOTES:
// Evic VTC mini X-MAX = 116


void setupScreen() {
    g.nextRefresh = uptime + 60;
    Display_SetOn(1);
    Display_Clear();
    Display_SetInverted(s.invertDisplay);

    uint8_t atomizerOn = Atomizer_IsOn();

    if (s.flipOnVape) {
        if (atomizerOn) {
	        if (!Display_IsFlipped()) {
	            Display_Flip();
	        }
        } else {
	        if (Display_IsFlipped()) {
	            Display_Flip();
	        }
        }
    }
}

void getTextDimensions(const Font_Info_t *font, char *text, struct layoutProperties *layout) {
    int8_t width = 0;
    int8_t height = 1;

    for(char c; (c = *text); text++) {
        if (c == '\n' || c == '\r') {
            height++;
            if (width && width > layout->W) {
                width -= font->kerning;
                layout->W = width;
            }
            width = 0;
            continue;
        }
        if (c >= font->startChar && c <= font->endChar)
            width += font->charInfo[c - font->startChar].width + font->kerning;
    }

    if(width && width > layout->W) {
        width -= font->kerning;
        layout->W = width;
    }

    if (height) {
        if (width) {
            layout->H = height * font->height;
        } else {
            layout->H = (height - 1) * font->height;
        }
    }
}

uint8_t textGetDimensions(void *priv, uint8_t *args, struct layoutProperties *layout) {
    struct textPriv *pData = (struct textPriv *)priv;
    const Font_Info_t *font = pData->font;

    if (pData->text) {
        free(pData->text);
        pData->text = NULL;
    }

    pData->text = calloc(pData->len, sizeof(char));
    pData->getText(pData->text, pData->len);

    getTextDimensions(font, pData->text, layout);
    return 0;
}

uint8_t textDraw(void *priv, uint8_t *args, struct layoutProperties *layout) {
    struct textPriv *pData = (struct textPriv *)priv;
    const Font_Info_t *font = pData->font;
    Display_PutText(layout->X, layout->Y, pData->text, font);
    free(pData->text);
    pData->text = NULL;
    return 0;
}

void fadeInTransition() {
    uint32_t now = uptime;
    uint32_t targetBrightness = s.screenBrightness;
    if (g.screenFadeInTime == 0) {
        g.screenFadeInTime = now + s.fadeInTime;
    }

    int chargeScreen = (g.charging && !g.pauseScreenOff && (g.screenOffTime < now));

    if (!g.pauseScreenOff && s.fadeOutTime >= g.screenOffTime - now && g.screenOffTime >= now) {

        // fade out if timing out
        g.currentBrightness = (((g.screenOffTime - now) * 1000 / s.fadeOutTime) * targetBrightness) / 1000;

    } else if (!chargeScreen && g.screenFadeInTime != 0 && now <= g.screenFadeInTime) {

        // fade in
        uint32_t startTime = g.screenFadeInTime - s.fadeInTime;
        g.currentBrightness = (((now - startTime) * 1000 / s.fadeInTime) * targetBrightness) / 1000;

    } else if (chargeScreen) {

        g.currentBrightness = 40;

    }

    bool needBrightness = g.currentBrightness <= targetBrightness;
    if (needBrightness && !chargeScreen) {
        // update animation time left
        g.screenFadeInTime = now +
                (s.fadeInTime - (((g.currentBrightness * 1000 / targetBrightness) * s.fadeInTime) / 1000));

    }

    Display_SetContrast(g.currentBrightness);
}

uint8_t parseAttributes(uint8_t *items, struct layoutProperties *lp) {
    uint8_t count = 0;
    uint8_t attr = 0;

    while ((attr = items[count++]) != ENDATTRGROUP) {
        switch (attr) {
            case ENDATTRGROUP:
                goto done;
            case ATTRDIMENSIONS:
                lp->W = items[count++];
                lp->H = items[count++];
                break;
            case ATTRMARGINS:
                lp->mXl = items[count++];
                lp->mXr = items[count++];
                lp->mYt = items[count++];
                lp->mYb = items[count++];
                break;
            case ATTRPADDING:
                lp->pXl = items[count++];
                lp->pXr = items[count++];
                lp->pYt = items[count++];
                lp->pYb = items[count++];
                break;
            case ATTRLOCATION:
                lp->X = items[count++];
                lp->Y = items[count++];
                break;
            case ATTRALIGN:
                lp->flags |= items[count++];
                break;
            default:
                goto done;
        }
    }

done:
    return count;
}

uint8_t getDimensions(uint8_t *items, struct layoutProperties *container) {
    uint8_t index = 0;
    uint8_t offset = 0;

    uint8_t havew = 0, haveh = 0;

    if (items[offset] == ATTRGROUP) {
        offset++;
        offset += parseAttributes(&items[offset], container);
    }

    havew = container->W > 0;
    haveh = container->H > 0;

    if (havew && haveh)
        goto done;

    while ((index = items[offset++]) != LD) {
        uint8_t lhavew = 0, lhaveh = 0;

        struct layoutProperties *object = calloc(1, sizeof(struct layoutProperties));
        struct layoutProperties *tempContainer = calloc(1, sizeof(struct layoutProperties));

        switch(index) {
            case LASTDRAWABLE:
            case ENDROWGROUP:
            case ENDCOLGROUP:
                free(object);
                free(tempContainer);
                goto done;
            case ATTRGROUP:
                free(object);
                free(tempContainer);
                continue;
            case COLGROUP:
                object->flags &= ~LEFTTORIGHT;
                offset += getDimensions(&items[offset], object);
                break;
            case ROWGROUP:
                object->flags = LEFTTORIGHT;
                offset += getDimensions(&items[offset], object);
                break;
            default:

                if (items[offset] == ATTRGROUP) {
                    offset++;
                    offset += parseAttributes(&items[offset], tempContainer);

                    lhavew = tempContainer->W != 0;
                    lhaveh = tempContainer->H != 0;
                }

                if (!lhavew || !lhaveh)
                    offset += drawables[index]->getDimensions(drawables[index]->priv, &items[offset], object);

                object->mXl = tempContainer->mXl;
                object->mXr = tempContainer->mXr;
                object->mYt = tempContainer->mYt;
                object->mYb = tempContainer->mYb;

                if (lhavew)
                    object->W = tempContainer->W;

                if (lhaveh)
                    object->H = tempContainer->H;
                break;
        }

        object->H += object->mYt + object->mYb;
        object->W += object->mXl + object->mXr;

        if (container->flags & LEFTTORIGHT) {
            if (!haveh)
                container->H = object->H > container->H ? object->H : container->H;

            if (!havew)
                container->W += object->W;
        } else {
            if (!havew)
                container->W = object->W > container->W ? object->W : container->W;

            if (!haveh)
                container->H += object->H;
        }
        free(object);
        free(tempContainer);
    }

done:
    container->W = container->W + container->mXl + container->mXr;
    container->H = container->H + container->mYt + container->mYb;
    return offset;
}

void drawBorder(struct layoutProperties *layout) {
    Display_PutLine(layout->X, layout->Y, layout->X, layout->Y + layout->H); //left
    Display_PutLine(layout->X + layout->W, layout->Y, layout->X + layout->W, layout->Y + layout->H); //right
    Display_PutLine(layout->X, layout->Y, layout->X + layout->W, layout->Y); // top
    Display_PutLine(layout->X, layout->Y + layout->H, layout->X + layout->W,  layout->Y + layout->H); //bottom
}

uint8_t drawItem(const struct displayItem *DI, uint8_t *args, struct layoutProperties *layout) {
    if (!DI) return 0;
    if (!DI->drawAt) return 0;

    uint8_t offset = 0;

    if (args[0] == ATTRGROUP) {
        offset++;
        offset += parseAttributes(&args[offset], layout);
    }

    offset += DI->drawAt(DI->priv, &args[offset], layout);

#ifdef SHOWOVERFLOW
    int8_t overflow = 0;

    if (layout->X < 0 ||
        layout->X > DISPLAY_WIDTH - 1 - layout->W ||
        layout->Y < 0 ||
        layout->Y > DISPLAY_HEIGHT - 1 - layout->H) {
        drawBorder(layout);
        overflow = 1;
    }


    if (overflow) {
#ifdef USBOVERFLOW
        W();
        writeUsb("Item overflowed bounds: %d x %d\n", DISPLAY_WIDTH, DISPLAY_HEIGHT);
        writeUsb("%s %d x %d @ %d,%d\n", DI->label, w, h, x, y );
#endif
    }
#endif

    return offset;
}

void calcObjectInLayout(struct layoutProperties *layout, struct layoutProperties *object) {
    int8_t x = layout->X + object->mXl;
    int8_t y = layout->Y + object->mYt;

    if (object->flags & VCENTER)
        y = layout->Y + (layout->H - object->H) / 2;

    if (object->flags & BOTTOM)
        y = layout->Y + layout->H - object->H - object->mYb;

    if (object->flags & CENTER)
        x = layout->X + (layout->W - object->W) / 2;

    if (object->flags & RIGHT)
        x = layout->X + layout->W - object->W - object->mXr;

    object->X = x;
    object->Y = y;
}

uint8_t drawItems (uint8_t *items, struct layoutProperties *container) {
    uint8_t index = 0;
    uint8_t offset = 0;

    if (items[offset] == ATTRGROUP) {
        offset++;
        offset += parseAttributes(&items[offset], container);
    }

    int8_t x = container->X + container->pXl,
           y = container->Y + container->pYt;

    int8_t xmax = container->X + container->W - container->pXr,
           ymax = container->Y + container->H - container->pYb;


    while ((index = items[offset++]) != LD) {
        struct layoutProperties *object = calloc(1, sizeof(struct layoutProperties));
        struct layoutProperties *layout = calloc(1, sizeof(struct layoutProperties));
        struct layoutProperties *tempContainer = calloc(1, sizeof(struct layoutProperties));

        layout->X = x;
        layout->Y = y;
        layout->W = xmax - x;
        layout->H = ymax - y;

        switch(index) {
            case LASTDRAWABLE:
            case ENDROWGROUP:
            case ENDCOLGROUP:
                free(object);
                free(layout);
                free(tempContainer);
                return offset;
            case COLGROUP:
                object->flags &= ~LEFTTORIGHT;
                object->H = layout->H;
                getDimensions(&items[offset], object);
                calcObjectInLayout(layout, object);
                offset += drawItems(&items[offset], object);
                break;
            case ROWGROUP:
                object->flags = LEFTTORIGHT;
                object->W = layout->W;
                getDimensions(&items[offset], object);
                calcObjectInLayout(layout, object);
                offset += drawItems(&items[offset], object);
                break;
            default:
                if (items[offset] == ATTRGROUP) {
                    offset++;
                    offset += parseAttributes(&items[offset], tempContainer);
                }

                object->flags = tempContainer->flags;

                object->mXl = tempContainer->mXl;
                object->mXr = tempContainer->mXr;
                object->mYt = tempContainer->mYt;
                object->mYb = tempContainer->mYb;

                object->pXl = tempContainer->pXl;
                object->pXr = tempContainer->pXr;
                object->pYt = tempContainer->pYt;
                object->pYb = tempContainer->pYb;

                if (!tempContainer->W || !tempContainer->H)
                    drawables[index]->getDimensions(drawables[index]->priv, &items[offset], object);

                if (tempContainer->W)
                    object->W = tempContainer->W;

                if (tempContainer->H)
                    object->H = tempContainer->H;

                if (!tempContainer->X || !tempContainer->Y)
                    calcObjectInLayout(layout, object);

                if (tempContainer->X)
                    object->X = tempContainer->X;

                if (tempContainer->Y)
                    object->Y = tempContainer->Y;

                offset += drawItem(drawables[index], &items[offset], object);
                break;
        }

        if(g.showBorders)
            drawBorder(layout);

        if (container->flags & LEFTTORIGHT)
            x += object->W;
        else
            y += object->H;

        free(object);
        free(layout);
        free(tempContainer);
    }
    return offset;

}

uint8_t drawScreen (uint8_t *items) {
    struct layoutProperties layout = {
        .X = 0,
        .Y = 0,

        .W = DISPLAY_WIDTH - 2,
        .H = DISPLAY_HEIGHT - 1,

        .mXl = 0,
        .mXr = 0,
        .mYt = 0,
        .mYb = 0,

        .pXl = 0,
        .pXr = 0,
        .pYt = 0,
        .pYb = 0,

        .flags = 0
    };

    return drawItems(items, &layout);
}