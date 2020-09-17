/**
 * Copyright (c) 2014 - 2020, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "sdk_common.h"
#include "nrf.h"
#include "nrf_esb.h"
#include "nrf_error.h"
#include "nrf_esb_error_codes.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "nrf_delay.h"
#include "app_util.h"
#include "nrfx_gpiote.h"
#include "nrf_drv_clock.h"

#include "timebase.h"
#include "esb.h"
#include "debug_swo.h"

#include "com_usb.h"


static uint8_t g_usb_rx_ready = 0;
static uint32_t test_flag = 0;

static void com_usb_event_handler(com_usb_evt_type_t evt_type)
{
    switch(evt_type){
    case COM_USB_EVT_PORT_OPENED:
        break;
    case COM_USB_EVT_PORT_CLOSED:
        break;
    case COM_USB_EVT_RX_DONE:
        g_usb_rx_ready = 1;
    }
}


static void button_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action){
    debug_swo_printf("BUTTON PRESSED\n");
    test_flag = 1;
}


void clocks_start( void )
{
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;

    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);

    nrf_drv_clock_init();
    nrf_drv_clock_lfclk_request(NULL);
}


void gpio_init( void )
{
    nrfx_gpiote_pin_t button_pin = NRF_GPIO_PIN_MAP(0, 11);
    nrfx_gpiote_in_config_t button_cfg = NRFX_GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
    button_cfg.pull = NRF_GPIO_PIN_PULLUP;
    nrfx_gpiote_init();
    nrfx_gpiote_in_init(button_pin, &button_cfg, button_handler);
    nrfx_gpiote_in_event_enable(button_pin, true);

    /* LED */
    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(1, 12));
    nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(1, 12));
}


int main(void)
{
    ret_code_t err_code;
    UNUSED_VARIABLE(err_code);

    debug_swo_init();

    gpio_init();

    debug_swo_printf("startup\n"),
    clocks_start();

    timebase_init();
    
    nrf_gpio_pin_set(NRF_GPIO_PIN_MAP(1, 12));

    com_usb_init(com_usb_event_handler);

    err_code = esb_init();

    uint8_t tx = 1;
    uint8_t rx[32] = {0};
    uint8_t rx_len = 0;
	while (true)
	{
        // if(g_usb_rx_ready == 1){
        //     g_usb_rx_ready = 0;
        //     com_usb_process();
        // }
        timebase_delay_ms(1000);
        nrf_gpio_pin_toggle(NRF_GPIO_PIN_MAP(1, 12));
        
        uint32_t millis = 0;
        timebase_get_tick(&millis, NULL);
    

        uint32_t bla = 1;
        if(bla == test_flag){
            test_flag = 0;


            int8_t result = esb_transmit_blocking(&tx,1,rx,&rx_len);

            debug_swo_printf("TX result: %i", result);
        }

    }
}
