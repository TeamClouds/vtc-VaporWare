#ifndef ____DEBUG
#define ____DEBUG
#include <USB_VirtualCOM.h>
#include <stdarg.h>

#define D() ;{char __buff[63];siprintf(__buff, "File: %s Line: %i\r\n", __FILE__, __LINE__);USB_VirtualCOM_SendString(__buff);};
#define W() ;{while(USB_VirtualCOM_GetAvailableSize() == 0){;}};

static inline void writeUsb(const char *format, ...) {
    va_list args;
    va_start(args, format);

    char buff[63] = {0};
    vsiprintf(buff, format, args);
    USB_VirtualCOM_SendString(buff);

    va_end(args);
}

#endif
