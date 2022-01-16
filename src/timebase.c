
#include "timebase.h"

#include "nrfx_systick.h"
#include "nrfx_timer.h"

const nrfx_timer_t g_timebase_timer = NRFX_TIMER_INSTANCE(1);

volatile uint8_t g_timeout_flag = 0;

static void timeout_timer_handler(nrf_timer_event_t event_type, void* p_context)
{
    if(event_type == NRF_TIMER_EVENT_COMPARE0){
        g_timeout_flag = 1;
        nrfx_timer_clear(&g_timebase_timer);
        nrfx_timer_disable(&g_timebase_timer);
    }
}
void timebase_init(void)
{
    nrfx_systick_init();
    nrfx_timer_config_t timer_cfg = NRFX_TIMER_DEFAULT_CONFIG;
    timer_cfg.frequency = NRF_TIMER_FREQ_1MHz;
    timer_cfg.mode = NRF_TIMER_MODE_TIMER;
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
    nrfx_timer_init(&g_timebase_timer, &timer_cfg, timeout_timer_handler);
    
    

}

void timebase_timeout_start(uint32_t timeout_ms)
{
    g_timeout_flag = 0;
    uint32_t timeout_ticks = nrfx_timer_ms_to_ticks(&g_timebase_timer, timeout_ms);
    nrfx_timer_extended_compare(&g_timebase_timer, NRF_TIMER_CC_CHANNEL0, timeout_ticks, NRF_TIMER_SHORT_COMPARE0_STOP_MASK, true);
    nrfx_timer_enable(&g_timebase_timer);
    
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