#include "nrf_gpio.h"

#include "led.h"

typedef enum {
    LED_POL_HIGH,   /**< LED on when pin is HIGH */
    LED_POL_LOW     /**< LED on when pin is LOW */
} polarity_t;

typedef struct {
    led_id_t id;
    uint32_t pin_number;
    uint8_t polarity;
} led_config_t;

led_config_t g_led_config[4] = {
    {LED_ID_STATUS, NRF_GPIO_PIN_MAP(1, 12), LED_POL_HIGH},
    {LED_ID_R, NRF_GPIO_PIN_MAP(0, 13), LED_POL_LOW},
    {LED_ID_G, NRF_GPIO_PIN_MAP(0, 14), LED_POL_LOW},
    {LED_ID_B, NRF_GPIO_PIN_MAP(0, 15), LED_POL_LOW},
};

void led_init(void)
{
    /* LED */
    for (uint8_t i = 0; i < LED_ID_MAX; i++)
    {
        nrf_gpio_cfg_output(g_led_config[i].pin_number);
        if(g_led_config[i].polarity == LED_POL_HIGH){
            nrf_gpio_pin_clear(g_led_config[i].pin_number);
        }else{
            nrf_gpio_pin_set(g_led_config[i].pin_number);
        }
    }
}

void led_set_state(led_id_t led_id, led_state_t state)
{
    if(led_id >= LED_ID_MAX)
    {
        return;
    }
    uint32_t led_pin = g_led_config[led_id].pin_number;

    if(state == LED_STATE_ON){
        // turn on
        if(g_led_config[led_id].polarity == LED_POL_HIGH){
            nrf_gpio_pin_set(led_pin);
        }else{
            nrf_gpio_pin_clear(led_pin);
        }
    } else {
        //turn off
        if(g_led_config[led_id].polarity == LED_POL_HIGH){
            nrf_gpio_pin_clear(led_pin);
        }else{
            nrf_gpio_pin_set(led_pin);
        }
    }
}