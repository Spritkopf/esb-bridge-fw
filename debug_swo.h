#ifndef DEBUG_SWO_H
#define DEBUG_SWO_H


#include <stdarg.h>

void debug_swo_init(void);

void debug_swo_printf(char* format, ...);

#endif // DEBUG_SWO_H