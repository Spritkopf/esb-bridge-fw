
#include "timebase.h"

#include "nrfx_systick.h"
#include "app_timer.h"

APP_TIMER_DEF(timeout_timer);

uint8_t g_timeout_flag = 0;

static void timeout_timer_handler(void* p_context)
{
    g_timeout_flag = 1;
    app_timer_stop(timeout_timer);
}
void timebase_init(void)
{
    nrfx_systick_init();

    app_timer_init();
    app_timer_create(&timeout_timer, APP_TIMER_MODE_SINGLE_SHOT, timeout_timer_handler);
    

}

uint8_t timebase_timeout_check_values(uint32_t timeout_ms, void* value1, void* value2)
{
    uint8_t result = 1;
    g_timeout_flag = 0;
    app_timer_start(timeout_timer, APP_TIMER_TICKS(timeout_ms), NULL);

    while(*(uint32_t*)value1 != *(uint32_t*)value2){
        if(g_timeout_flag == 1){
            g_timeout_flag = 0;
            result = 0;
            break;
        }
    }

    app_timer_stop(timeout_timer);
    return (result);
}

void timebase_timeout_start(uint32_t timeout_ms)
{
    g_timeout_flag = 0;
    app_timer_start(timeout_timer, APP_TIMER_TICKS(timeout_ms), NULL);
}

uint8_t timebase_timeout_check(void)
{
    if(g_timeout_flag == 1)
    {  
        g_timeout_flag = 0;
        return (1);
    }

    return (0);
}

void timebase_get_tick(uint32_t *p_millis, uint32_t *p_micros)
{
    
}

void timebase_delay_ms(uint32_t millis)
{
    nrfx_systick_delay_ms(millis);
}