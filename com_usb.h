#ifndef COM_USB_H_
#define COM_USB_H_

#include <stdint.h>

typedef enum{
    COM_USB_EVT_PORT_OPENED,
    COM_USB_EVT_PORT_CLOSED
} com_usb_evt_type_t;

typedef void (*com_usb_evt_callback_t)(com_usb_evt_type_t evt_type);


void com_usb_init(com_usb_evt_callback_t com_usb_evt_callback);

int8_t com_usb_tx(uint8_t *data, uint32_t len);


#endif // #ifndef COM_USB_H_