#include "com_usb.h"
#include "nrf_error.h"
#include "nrf_drv_usbd.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_power.h"
#include "app_usbd_core.h"
#include "app_usbd.h"
#include "app_usbd_string_desc.h"
#include "app_usbd_cdc_acm.h"
#include "app_usbd_serial_num.h"

#define CDC_ACM_STARTUP_DELAY_MS 100 

#define CDC_ACM_COMM_INTERFACE 0
#define CDC_ACM_COMM_EPIN NRF_DRV_USBD_EPIN2

#define CDC_ACM_DATA_INTERFACE 1
#define CDC_ACM_DATA_EPIN NRF_DRV_USBD_EPIN1
#define CDC_ACM_DATA_EPOUT NRF_DRV_USBD_EPOUT1

#define RX_BUFFER_SIZE 64


static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const *p_inst, app_usbd_cdc_acm_user_event_t event);

static void usbd_user_ev_handler(app_usbd_event_type_t event);

static com_usb_evt_callback_t g_com_usb_evt_callback = NULL;

/**
 * @brief CDC_ACM class instance
 * */
APP_USBD_CDC_ACM_GLOBAL_DEF(m_app_cdc_acm, cdc_acm_user_ev_handler, CDC_ACM_COMM_INTERFACE, CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN, CDC_ACM_DATA_EPIN, CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_AT_V250);

static const app_usbd_config_t usbd_config = {.ev_state_proc = usbd_user_ev_handler};

static uint8_t m_rx_buffer[RX_BUFFER_SIZE];


static void usbd_user_ev_handler(app_usbd_event_type_t event)
{
    switch (event) {
    case APP_USBD_EVT_DRV_SUSPEND:
        break;
    case APP_USBD_EVT_DRV_RESUME:
        break;
    case APP_USBD_EVT_STARTED:
        break;
    case APP_USBD_EVT_STOPPED:
        app_usbd_disable();
        break;
    case APP_USBD_EVT_POWER_DETECTED:

        if (!nrf_drv_usbd_is_enabled()) {
            app_usbd_enable();
        }
        break;
    case APP_USBD_EVT_POWER_REMOVED:
        app_usbd_stop();
        break;
    case APP_USBD_EVT_POWER_READY:
        app_usbd_start();
        break;
    default:
        break;
    }
}

static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const *p_inst, app_usbd_cdc_acm_user_event_t event)
{
    app_usbd_cdc_acm_t const *p_cdc_acm = app_usbd_cdc_acm_class_get(p_inst);

    switch (event) {
    case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN: {
        /* Setup first transfer */
        ret_code_t ret = app_usbd_cdc_acm_read_any(&m_app_cdc_acm, m_rx_buffer, RX_BUFFER_SIZE);
        UNUSED_VARIABLE(ret);

        if (g_com_usb_evt_callback != NULL) {
            g_com_usb_evt_callback(COM_USB_EVT_PORT_OPENED);
        }
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE: {
        if (g_com_usb_evt_callback != NULL) {
            g_com_usb_evt_callback(COM_USB_EVT_PORT_CLOSED);
        }

        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
        break;
    case APP_USBD_CDC_ACM_USER_EVT_RX_DONE: {
        ret_code_t ret;
        UNUSED_VARIABLE(ret);

        /* Get amount of data transfered */
        size_t size = app_usbd_cdc_acm_rx_size(p_cdc_acm);

        UNUSED_VARIABLE(size);
        /* TODO: HANDLE DATA HERE */

        /* check if received message did not fit into rx buffer and get the rest if necessary */

        size_t bytes_left = app_usbd_cdc_acm_bytes_stored(p_cdc_acm);
        while (bytes_left) {
            uint32_t bytes_to_read = MIN(bytes_left, RX_BUFFER_SIZE);

            ret = app_usbd_cdc_acm_read_any(&m_app_cdc_acm, m_rx_buffer, bytes_to_read);
            /* TODO: HANDLE DATA HERE */

            bytes_left -= bytes_to_read;
        }

        /* prepare next transfer */
        ret = app_usbd_cdc_acm_read_any(&m_app_cdc_acm, m_rx_buffer, RX_BUFFER_SIZE);
        break;
    }
    default:
        break;
    }
}

/******************************************************************************
 *                              GLOBAL FUNCTIONS                              *
 ******************************************************************************/

void com_usb_init(com_usb_evt_callback_t evt_callback)
{
    ret_code_t ret;
    UNUSED_VARIABLE(ret);

    /* Because of the power-on behavior of the Xenon board, in some cases the
       USB inteface does not come up properly when initialized immediately.
       A short delay before initialization solves this issue */
       
    //tmb_delay_ms(CDC_ACM_STARTUP_DELAY_MS);
    
    ret = nrf_drv_clock_init();

    app_usbd_serial_num_generate();

    /* Initialize USB */
    app_usbd_init(&usbd_config);
    app_usbd_class_inst_t const *class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&m_app_cdc_acm);
    app_usbd_class_append(class_cdc_acm);

    app_usbd_enable();
    app_usbd_start();

    if(evt_callback != NULL){
        g_com_usb_evt_callback = evt_callback;
    }
}


int8_t com_usb_tx(uint8_t *data, uint32_t len)
{
    int8_t result;

    result = app_usbd_cdc_acm_write(&m_app_cdc_acm, (const void *)data, (size_t)len);

    return result;
}