#pragma once

typedef enum {
    LED_ID_STATUS = 0, 
    LED_ID_R = 1,
    LED_ID_G = 2,
    LED_ID_B = 3,
    LED_ID_MAX = 4
} led_id_t;

typedef enum {
    LED_STATE_OFF, 
    LED_STATE_ON
} led_state_t;

/**
 * @brief Initialize LEDs
 * 
 */
void led_init(void);

/**
 * @brief Set state of an LED
 * 
 * @param led_id 
 * @param state 
 */
void led_set_state(led_id_t led_id, led_state_t state);