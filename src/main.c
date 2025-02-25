
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "sdk_common.h"
#include "nrf.h"
#include "nrf_esb.h"
#include "nrf_error.h"
#include "nrf_esb_error_codes.h"
#include "boards.h"
#include "nrf_delay.h"
#include "app_util.h"
#include "nrfx_gpiote.h"
#include "nrf_drv_clock.h"

#include "timebase.h"
#include "esb.h"
#include "debug_swo.h"
#include "led.h"

#include "com_usb.h"
#include "com_usb_commands.h"


static uint8_t g_usb_rx_ready = 0;



/* Handler for  USB events*/
static void com_usb_event_handler(com_usb_evt_type_t evt_type)
{
    switch(evt_type){
    case COM_USB_EVT_PORT_OPENED:
        debug_swo_printf("Usb Connected\n");
        nrf_gpio_pin_set(NRF_GPIO_PIN_MAP(1, 12));
        break;
    case COM_USB_EVT_PORT_CLOSED:
        debug_swo_printf("Usb Disconnected\n");
        nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(1, 12));
        break;
    case COM_USB_EVT_RX_DONE:
        g_usb_rx_ready = 1;
    }
}

/* Handler for the user button, used for testing */
static void button_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action){
    debug_swo_printf("BUTTON PRESSED\n");

    // For client test: Send a USB message with CMD_IRQ to the host
    usb_message_t msg = {
        .cmd = CMD_IRQ,
        .error = E_OK,
        .payload_len=5 + 3,
    };
    msg.payload[0] = 0xde;
    msg.payload[1] = 0xad;
    msg.payload[2] = 0xbe;
    msg.payload[3] = 0xef;
    msg.payload[4] = 1;

    msg.payload[5] = 0xAA;
    msg.payload[6] = 0xBB;
    msg.payload[7] = 0xCC;
    com_usb_transmit(&msg);
}


/* Configure the clock */
void clocks_start( void )
{
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;

    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);

    // needed for the app_timer module
    nrf_drv_clock_init();
    nrf_drv_clock_lfclk_request(NULL);
}


/* Setup GPIO for the user button*/
void gpio_init( void )
{
    nrfx_gpiote_pin_t button_pin = NRF_GPIO_PIN_MAP(0, 11);
    nrfx_gpiote_in_config_t button_cfg = NRFX_GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
    button_cfg.pull = NRF_GPIO_PIN_PULLUP;
    nrfx_gpiote_init();
    nrfx_gpiote_in_init(button_pin, &button_cfg, button_handler);
    nrfx_gpiote_in_event_enable(button_pin, true);

    
}


/* Enable PCB Antenna
 * TODO: THis is specific to the "Particle" board used for development, move to board-support layer
*/
void rf_antenna_init(void)
{
    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0, 24));
    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0, 25));

    /* Enable PCB antenna */
    nrf_gpio_pin_set(NRF_GPIO_PIN_MAP(0, 24));
    nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0, 25));
}


int main(void)
{
    ret_code_t err_code;
    UNUSED_VARIABLE(err_code);

    debug_swo_init();

    gpio_init();

    debug_swo_printf("startup\n");
    clocks_start();

    timebase_init();
    
    /* turn user LED on*/
    led_init();
    led_set_state(LED_ID_STATUS, LED_STATE_ON);
    rf_antenna_init();

    com_usb_init(com_usb_event_handler);
    (void)com_usb_event_handler,

    err_code = esb_init();

	while (true)
	{
        if(g_usb_rx_ready == 1){
            g_usb_rx_ready = 0;
            com_usb_process();
        }
    }
}
