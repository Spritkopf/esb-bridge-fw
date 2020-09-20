#ifndef COM_USB_H_
#define COM_USB_H_

#include <stdint.h>

#define USB_PROTOCOL_SYNC_BYTE   0x69 /* arbitrary byte for synchronisation ('i')*/
#define USB_PROTOCOL_PACKET_SIZE        64
#define USB_PROTOCOL_CHECKSUM_SIZE      2
#define USB_PROTOCOL_HEADER_SIZE        4
#define USB_PROTOCOL_MIN_LENGTH         (USB_PROTOCOL_HEADER_SIZE + USB_PROTOCOL_CHECKSUM_SIZE)
#define USB_PROTOCOL_MAX_PAYLOAD_LENGTH (USB_PROTOCOL_PACKET_SIZE - USB_PROTOCOL_MIN_LENGTH)

typedef enum
{
    E_OK      = 0x00,   /* no error */
    E_NO_CMD  = 0x10,   /* command not found */
    E_PL_LEN  = 0x40,   /* payload length too large or too short */
    E_ESB     = 0x80,   /* Failed to execute ESB function */ 
} usb_error_t;

typedef struct 
{
    uint8_t sync_byte;
    uint8_t cmd;
    uint8_t error;
    uint8_t payload_len;
    uint8_t payload[USB_PROTOCOL_MAX_PAYLOAD_LENGTH];
    uint16_t crc16;
} usb_message_t;

typedef enum{
    COM_USB_EVT_PORT_OPENED,
    COM_USB_EVT_PORT_CLOSED,
    COM_USB_EVT_RX_DONE,
} com_usb_evt_type_t;

typedef void (*com_usb_evt_callback_t)(com_usb_evt_type_t evt_type);

/* initialize usb CDC */
void com_usb_init(com_usb_evt_callback_t com_usb_evt_callback);

/* check if command is available and process it */
void com_usb_process(void);

/* send a message */
void com_usb_transmit(usb_message_t* message);
#endif // #ifndef COM_USB_H_