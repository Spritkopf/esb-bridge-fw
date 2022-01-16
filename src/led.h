#pragma once

#include <stdint.h>

typedef enum {
    LED_ID_STATUS = 0,  /**< Status LED */
    LED_ID_R = 1,       /**< Red LED */
    LED_ID_G = 2,       /**< Green LED */
    LED_ID_B = 3,       /**< Blue LED */
    LED_ID_MAX = 4       /**< Number of vailable LEDs */
} led_id_t;

typedef enum {
    LED_STATE_OFF, /**< LED is OFF */
    LED_STATE_ON   /**< LED is ON */
} led_state_t;

/**
 * @brief Initialize LEDs
 * 
 */
void led_init(void);

/**
 * @brief Set state of an LED
 * 
 * @param led_id LED ID, see ::led_id_t
 * @param state LED state, see ::led_state_t
 */
void led_set_state(led_id_t led_id, led_state_t state);

/**
 * @brief Flash the LED for a specified duration
 * 
 * @param led_id LED ID, see ::led_id_t
 * @param duration_ms flash duration in milliseconds
 */
void led_flash_once(led_id_t led_id, uint32_t duration_ms);