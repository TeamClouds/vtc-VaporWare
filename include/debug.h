#ifndef ____DEBUG
#define ____DEBUG
#include <USB_VirtualCOM.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

static inline void writeUsb(const char *format, ...) {
    va_list args;
    va_start(args, format);

    char buff[63] = {0};
    vsiprintf(buff, format, args);
    USB_VirtualCOM_SendString(buff);

    va_end(args);
}

#define D() do {writeUsb("File: %s Line: %i\r\n", __FILE__, __LINE__);} while(0)
#define W() do {;} while(USB_VirtualCOM_GetAvailableSize() == 0)

#endif
