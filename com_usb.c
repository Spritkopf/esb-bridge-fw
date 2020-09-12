#include "nrf_error.h"
#include "nrf_drv_usbd.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_power.h"
#include "app_usbd_core.h"
#include "app_usbd.h"
#include "app_usbd_string_desc.h"
#include "app_usbd_cdc_acm.h"
#include "app_usbd_serial_num.h"

#include "com_usb.h"
#include "com_usb_commands.h"
#include "crc16_ccitt.h"

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

static uint8_t usb_tx_buffer[USB_PROTOCOL_PACKET_SIZE];
static uint8_t usb_rx_buffer[USB_PROTOCOL_PACKET_SIZE];
static uint8_t usb_rx_available = 0;

/**
 * @brief CDC_ACM class instance
 * */
APP_USBD_CDC_ACM_GLOBAL_DEF(m_app_cdc_acm, cdc_acm_user_ev_handler, CDC_ACM_COMM_INTERFACE, CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN, CDC_ACM_DATA_EPIN, CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_AT_V250);

static const app_usbd_config_t usbd_config = {.ev_state_proc = usbd_user_ev_handler};

static uint8_t m_rx_buffer[RX_BUFFER_SIZE];


static int8_t com_usb_tx(uint8_t *data, uint32_t len)
{
    int8_t result = app_usbd_cdc_acm_write(&m_app_cdc_acm, (const void *)data, (size_t)len);

    return result;
}


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

        /* check length, sync byte */
        if((usb_rx_available == 0) && (size == USB_PROTOCOL_PACKET_SIZE) && (m_rx_buffer[0] == USB_PROTOCOL_SYNC_BYTE)){
            memcpy(usb_rx_buffer, m_rx_buffer, RX_BUFFER_SIZE);
            usb_rx_available = 1;
        }

        /* prepare next transfer */
        ret = app_usbd_cdc_acm_read_any(&m_app_cdc_acm, m_rx_buffer, RX_BUFFER_SIZE);

        if(usb_rx_available == 1){
            g_com_usb_evt_callback(COM_USB_EVT_RX_DONE);
        }
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

void com_usb_process(void)
{
    usb_message_t received_message = {0};
    usb_message_t answer_message = {0};
    cmd_table_item_t* cmd_table_item;

    if(usb_rx_available == 1)
    {
        usb_rx_available = 0;
        
        /* check CRC16 */
        uint16_t crc = crc16_ccitt((void*)usb_rx_buffer, USB_PROTOCOL_PACKET_SIZE-USB_PROTOCOL_CHECKSUM_SIZE);
        received_message.crc16 = *(uint16_t*)&usb_rx_buffer[USB_PROTOCOL_PACKET_SIZE-USB_PROTOCOL_CHECKSUM_SIZE];
        if(crc != received_message.crc16)
        {
            /* CRC error, ignore message */
            return;
        }

        received_message.cmd = (uint8_t)usb_rx_buffer[1];
        received_message.payload_len = (uint8_t)usb_rx_buffer[3];

        /* by default, answer will have same cmd id and no error */
        answer_message.cmd = received_message.cmd;
        answer_message.error = E_OK;
        
        if(received_message.payload_len > USB_PROTOCOL_MAX_PAYLOAD_LENGTH)
        {
            answer_message.error = E_PL_LEN;
        }
        else
        {
            for(uint8_t i = 0; i < received_message.payload_len; i++)
            {
                received_message.payload[i] = (uint8_t)usb_rx_buffer[i+USB_PROTOCOL_HEADER_SIZE];
            }

            /* lookup command */
            cmd_table_item = com_usb_commands_lookup(received_message.cmd, received_message.payload_len);
            if(cmd_table_item != NULL)
            {
                /* execute the function associated to the command */
                cmd_table_item->cmd_fct_pnt(&received_message, &answer_message);
            }
            else
            {
                /* command not found - send ERROR */
                
                answer_message.error = E_NO_CMD;
            }
        }

        /* send the answer */
        com_usb_transmit(&answer_message);

    }
}


void com_usb_transmit(usb_message_t* message)
{
    uint16_t crc = 0;
    memset(usb_tx_buffer, 0, sizeof(usb_tx_buffer));

    usb_tx_buffer[0] = USB_PROTOCOL_SYNC_BYTE;
    usb_tx_buffer[1] = message->cmd;
    usb_tx_buffer[2] = message->error;
    usb_tx_buffer[3] = message->payload_len;
    memcpy(&usb_tx_buffer[4], message->payload, 58);
    crc = crc16_ccitt(usb_tx_buffer, USB_PROTOCOL_PACKET_SIZE-USB_PROTOCOL_CHECKSUM_SIZE);
    usb_tx_buffer[62] = (uint8_t)(crc & 0xFF);
    usb_tx_buffer[63] = (uint8_t)((crc>>8) & 0xFF);

    /* send data */
   
    com_usb_tx(&usb_tx_buffer[0],USB_PROTOCOL_PACKET_SIZE);
}