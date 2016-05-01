#ifndef VAPORWARE_FONT_H
#define VAPORWARE_FONT_H

#include <Font_Data.h>
#include "Font_Large.h"
#include "Font_Medium.h"
#include "Font_Small.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FONT_LARGE (&Font_Large_FontInfo)
#define FONT_MEDIUM (&Font_Medium_FontInfo)
#define FONT_SMALL (&Font_Small_FontInfo)

#ifdef __cplusplus
}
#endif

#endif
