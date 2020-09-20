#include <stddef.h>
#include <string.h>
#include "com_usb_commands.h"
#include "com_usb.h"
#include "nrf.h"
#include "nrf_esb.h"
#include "nrf_error.h"
#include "debug_swo.h"
#include "esb.h"

#define PAYLOAD_LEN_DYNAMIC 0xFF


void cmd_fct_test(const usb_message_t* message, usb_message_t* answer)
{
    answer->cmd = message->cmd;
    answer->payload_len = 0;
    
    debug_swo_printf("Test command: [");

    for(uint8_t i=0; i< message->payload_len; i++){
        debug_swo_printf(" %02X ", message->payload[i]);
    }

    debug_swo_printf("]\n");
    return;
}

/* Get FW Version
 * payload length must be 0
 * answer payload: answer payload
 * answer error: E_OK if OK, otherwise E_ESB
 */
void cmd_fct_get_version(const usb_message_t* message, usb_message_t* answer)
{
    answer->payload_len = 3;
    answer->payload[0] = 0;
    answer->payload[1] = 1;
    answer->payload[2] = 0;
    
    return;
}


/* Transfer a ESB package and return the answer payload
 * The message must contain at least 6 bytes:
 * payload[0]:    Target address
 * payload[1-N]:    payload to send (N max 32)
 * payload length determines how much bytes will be written (N-5, because of target address)
 * answer payload: answer payload
 * answer error: E_OK if OK, otherwise E_ESB
 */
void cmd_fct_transfer(const usb_message_t* message, usb_message_t* answer)
{
    if((message->payload_len < 2) || (message->payload_len > NRF_ESB_MAX_PAYLOAD_LENGTH +1))
    {
        /* message must contain at least 2 bytes */
        answer->error = E_PL_LEN;
    }
    else
    {
        nrf_esb_payload_t tx_payload = {
            .pipe = 0,
            .length = message->payload_len-1,
            .noack = false
        };
        memcpy(tx_payload.data, &(message->payload[1]), tx_payload.length);
        
        if (nrf_esb_write_payload(&tx_payload) == NRF_SUCCESS)
        {
            debug_swo_printf("Sending success\n");
            answer->error = E_OK;
        }
        else
        {
            debug_swo_printf("Sending packet failed\n");
            answer->error = E_ESB;
        }
    }
    

    return;
}

/* Set address of TX Pipeline
 * payload:    5 bytes TX pipeline address
 * No answer payload
 */
void cmd_fct_set_tx_addr(const usb_message_t* message, usb_message_t* answer)
{
    answer->error = 0;    
    if(esb_set_tx_address(message->payload) != 0){
        answer->error = E_ESB;    
    }

    return;
}

/* Set address of specific RX Pipeline
 * paxload[0]: rx pipeline number
 * payload[1-5]:    5 bytes RX pipeline address
 * No answer payload
 */
void cmd_fct_set_rx_addr(const usb_message_t* message, usb_message_t* answer)
{
    /* coming soon */

    answer->error = 0;    

    return;
}

/*!
 * \brief Command table
 */
cmd_table_item_t cmd_table[] =
{
    /* COMMAND_ID       PAYLOAD_SIZE          FUNCTION_POINTER*/
    {CMD_TEST,          PAYLOAD_LEN_DYNAMIC,  cmd_fct_test},
    {CMD_VERSION,       0,                    cmd_fct_get_version},
    //{CMD_TRANSFER,      PAYLOAD_LEN_DYNAMIC,  cmd_fct_transfer},
    {CMD_SET_TX_ADDR,   5,                    cmd_fct_set_tx_addr},

    
    /* last entry NULL-terminator */
    {0,                 0,                    NULL}
};


/* Helper functions */
/*!
 * \brief lookup command in table
 * \returns pointer to command table entry if found, otherwise NULL
 */
cmd_table_item_t* com_usb_commands_lookup(uint8_t cmd_id, uint8_t payload_len)
{
    cmd_table_item_t* cmd_table_item_pnt = NULL;
    uint8_t index;

    for(index = 0; cmd_table[index].cmd_fct_pnt != NULL; index++)
    {   
        if (cmd_id == cmd_table[index].command_id)
        {
            if( (cmd_table[index].payload_size == payload_len) ||
                (cmd_table[index].payload_size == PAYLOAD_LEN_DYNAMIC))
            {
                /* return pointer to entry */
                cmd_table_item_pnt = &(cmd_table[index]);
                break;             
            }
            else
            {
                /* no match, stop now */
                break;
            }
        }
    }

    return (cmd_table_item_pnt);
}


