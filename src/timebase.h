#ifndef TIMEBASE_H_
#define TIMEBASE_H_

#include <stdint.h>

/* initialize timebase timer (RTC) */
void timebase_init(void);

/* get timebase tick (milliseconds since initialization) 
 * returns systick milliseconds and microseconds seperately
 * */
void timebase_get_tick(uint32_t *p_millis, uint32_t *p_micros);

/* start a timeout timer */
void timebase_timeout_start(uint32_t timeout_ms);

/* check if the timeout timer has elapsed */
uint8_t timebase_timeout_check(void);

/* blocking delay for amount of milliseconds */
void timebase_delay_ms(uint32_t millis);


#endif