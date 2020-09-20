
#ifndef COM_USB_COMMANDS_H_
#define COM_USB_COMMANDS_H_

#include "com_usb.h"

typedef enum
{
    CMD_VERSION     = 0x10,   /* Get firmware version */
    CMD_SET_TX_ADDR = 0x20,
    CMD_TEST        = 0x61,   /* test command, do not use */
    CMD_IRQ         = 0x80,   /* interrupt callback, only from device to host */
} usb_cmd_t;

/*
 * \brief Definition of commands
 */
typedef struct
{
    uint8_t command_id;         /*!< command id */
    uint8_t payload_size;       /*!<  expected payload size */
    void (*cmd_fct_pnt)(const usb_message_t* message, usb_message_t* answer);      /*!< pointer to command */
} cmd_table_item_t;


/*!
 * \brief   Command table
 */
extern cmd_table_item_t cmd_table[];



cmd_table_item_t* com_usb_commands_lookup(uint8_t cmd_id, uint8_t payload_len);

#endif /* COM_USB_COMMANDS_H_ */
