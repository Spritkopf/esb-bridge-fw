#include <stddef.h>
#include <string.h>
#include "com_usb_commands.h"
#include "com_usb.h"

#include "debug_swo.h"

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

void cmd_fct_get_version(const usb_message_t* message, usb_message_t* answer)
{
    answer->payload_len = 3;
    answer->payload[0] = 1;
    answer->payload[1] = 0;
    answer->payload[2] = 0;
    
    return;
}


/* Transfer a ESB package and return the answer payload
 * The message must contain at least 6 bytes:
 * payload[0]:    Target address
 * payload[1-N]:    payload to send (5 <= N <= 36)
 * payload length determines how much bytes will be written (N-5, because of target address)
 * answer payload: answer payload
 */
void cmd_fct_transfer(const usb_message_t* message, usb_message_t* answer)
{
    answer->error = 0;    
  #if 0  
    if(message->payload_len < 2)
    {
        /* message must contain at least 2 bytes */
        answer->error = E_PL_LEN;
    }
    else
    {
        result = nrf_com_transfer_blocking(&(message->payload[0]), &(message->payload[5]),message->payload_len-5,answer->payload,&(answer->payload_len));
        //result = ptx_transfer_blocking(message->payload,message->payload_len,answer->payload,&(answer->payload_len));

        if(result != 0)
        {
            answer->error = result*(-1);
        }    
    }
    #endif
    

    return;
}

/* Set address of TX Pipeline
 * payload:    5 bytes TX pipeline address
 * No answer payload
 */
void cmd_fct_set_tx_addr(const usb_message_t* message, usb_message_t* answer)
{
    //nrf_ptx_set_address(message->payload);

    answer->error = 0;    

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
    //{CMD_SET_TX_ADDR,   5,                    cmd_fct_set_tx_addr},

    
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


