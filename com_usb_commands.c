#include <stddef.h>
#include <string.h>
#include "com_usb_commands.h"
#include "com_usb.h"
#include "nrf.h"
#include "nrf_esb.h"
#include "nrf_error.h"
#include "debug_swo.h"
#include "esb.h"
#include "timebase.h"

#define PAYLOAD_LEN_DYNAMIC 0xFF


#define CMD_E_ESB           0x81
#define CMD_E_ESB_TIMEOUT   0x82

static volatile uint8_t g_esb_answer_received = 0;
static volatile uint8_t g_esb_answer_payload[NRF_ESB_MAX_PAYLOAD_LENGTH];
static volatile uint8_t g_esb_answer_payload_len = 0;

static void answer_callback(uint8_t *payload, uint8_t payload_length)
{
    debug_swo_printf("Got an answer!: [ ");
    for(uint8_t i = 0; i<payload_length; i++){
        debug_swo_printf("%02X ", payload[i]);  

    }
    debug_swo_printf("]\n");  

    memcpy((uint8_t*)g_esb_answer_payload, payload, payload_length);
    g_esb_answer_payload_len = payload_length;
    g_esb_answer_received = 1;
}

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
    debug_swo_printf("Cmd: Get FW Version\n");
    answer->payload_len = 3;
    answer->payload[0] = 0;
    answer->payload[1] = 3;
    answer->payload[2] = 0;
    
    return;
}


/* Transfer a ESB package and return the answer payload
 * The message payload has the target address in the first 5 bytes, afterwards the payload
 * minimum payload size: 6 bytes (address + at least 1 payload byte)
 * answer payload: answer payload
 * answer error: E_OK if OK, otherwise E_ESB
 * 
 */
void cmd_fct_transfer(const usb_message_t* message, usb_message_t* answer)
{
    if((message->payload_len < 6) || (message->payload_len > NRF_ESB_MAX_PAYLOAD_LENGTH))
    {
        /* invalid payload length */
        answer->error = E_PL_LEN;
    }
    else
    {
        esb_set_pipeline_address(ESB_PIPE_0, message->payload);
        int8_t result = esb_send_packet(ESB_PIPE_0, &(message->payload[5]), (message->payload_len)-5);
        
        if (result == ESB_ERR_OK)
        {
            esb_start_listening(ESB_PIPE_0, answer_callback);
            debug_swo_printf("Sending success\n");
            answer->error = E_OK;
        } else {
            debug_swo_printf("Sending packet failed\n");
            answer->error = CMD_E_ESB;
        }
        timebase_timeout_start(1000);
        while(g_esb_answer_received==0){
            if(timebase_timeout_check()==1){
                debug_swo_printf("Timeout waiting for answer\n");
                answer->error = CMD_E_ESB_TIMEOUT;
                break;
            }
        }
        memcpy(answer->payload, (uint8_t*)g_esb_answer_payload, g_esb_answer_payload_len);
        answer->payload_len = g_esb_answer_payload_len;
        g_esb_answer_received = 0;
        esb_stop_listening(ESB_PIPE_0);
    }
    return;
}


/* Transmit a ESB package
 * The message payload must be >=1 and <= 32 :
 * answer error: E_OK if OK, otherwise E_ESB
 * 
 * Note: set TX address beforehand with command CMD_SET_TX_ADDR
 */
void cmd_fct_send(const usb_message_t* message, usb_message_t* answer)
{
    answer->error = 0;    
    if(esb_send_packet(ESB_PIPE_0, message->payload, message->payload_len) != 0){
        answer->error = CMD_E_ESB;    
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
    esb_set_pipeline_address(ESB_PIPE_0, message->payload);
    
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
    {CMD_TRANSFER,      PAYLOAD_LEN_DYNAMIC,  cmd_fct_transfer},
    {CMD_SET_TX_ADDR,   5,                    cmd_fct_set_tx_addr},
    {CMD_SEND,          PAYLOAD_LEN_DYNAMIC,  cmd_fct_send},

    
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


