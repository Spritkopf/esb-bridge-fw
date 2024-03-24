

#include <stdint.h>
#include <stdio.h>
#include "nrf.h"
#include "debug_swo.h"

#define DEBUG_SWO_BUFFER_SIZE 255

void debug_swo_init(void)
{
#ifdef DEBUG
    NRF_CLOCK->TRACECONFIG = (NRF_CLOCK->TRACECONFIG & ~CLOCK_TRACECONFIG_TRACEPORTSPEED_Msk) |
		(CLOCK_TRACECONFIG_TRACEPORTSPEED_16MHz << CLOCK_TRACECONFIG_TRACEPORTSPEED_Pos);
	ITM->TCR |= 1;
	ITM->TER |= 1;
#endif    
}

void debug_swo_printf(char* format, ...)
{
#ifdef DEBUG
    char buf[DEBUG_SWO_BUFFER_SIZE];
	va_list argptr;
    va_start(argptr, format);
    uint32_t len = vsnprintf(buf, DEBUG_SWO_BUFFER_SIZE, format, argptr);
    va_end(argptr);
	
    for(uint32_t i = 0; i < len; i++){
        ITM_SendChar(buf[i]);
    }
#endif
}