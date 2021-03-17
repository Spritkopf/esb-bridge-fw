#include <string.h>
#include "nrf.h"
#include "nrf_esb.h"
#include "nrf_error.h"
#include "nrf_delay.h"

#include "esb.h"
#include "timebase.h"
#include "debug_swo.h"

#define ESB_CHECK_PIPE_PARAM(pipe)    do{if(pipe>=ESB_PIPE_NUM){return(ESB_ERR_PARAM);}}while(0)
#define ESB_CHECK_NULL_PARAM(param)   do{if(param==NULL){return(ESB_ERR_PARAM);}}while(0)

#define ESB_DEFAULT_CHANNEL 40

#define ESB_PIPE_DIRECT_COMM    ESB_PIPE_0
#define ESB_PIPE_LISTENING      ESB_PIPE_1

static nrf_esb_payload_t        rx_payload;
static nrf_esb_payload_t        tx_payload;

static volatile uint8_t rx_payload_available = 0;

static nrf_esb_config_t g_nrf_esb_config = NRF_ESB_DEFAULT_CONFIG;

static volatile uint8_t g_initialized = 0;
static volatile uint8_t g_tx_busy = 0;

static esb_listener_callback_t g_listener_callbacks[ESB_PIPE_NUM] = {NULL, NULL};

static uint8_t g_pipe_addr[ESB_PIPE_NUM][5] = {{0xC2, 0xC2, 0xC2, 0xC2, 0x01}, {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}}
;
static int8_t esb_reinit(nrf_esb_mode_t esb_mode);

static void nrf_esb_event_handler(nrf_esb_evt_t const * p_event)
{
    switch (p_event->evt_id){
        case NRF_ESB_EVENT_TX_SUCCESS:
            debug_swo_printf("TX SUCCESS EVENT (%lu attempts)\n", p_event->tx_attempts);
            g_tx_busy = 0;
            esb_reinit(NRF_ESB_MODE_PRX);
            break;
        case NRF_ESB_EVENT_TX_FAILED:
            debug_swo_printf("TX FAILED EVENT\n");
            g_tx_busy = 0;
            break;
        case NRF_ESB_EVENT_RX_RECEIVED:
            debug_swo_printf("RX RECEIVED EVENT\n");
            while (nrf_esb_read_rx_payload(&rx_payload) == NRF_SUCCESS){
                if (rx_payload.length > 0){
                    if(rx_payload.pipe < ESB_PIPE_NUM){
                        if(g_listener_callbacks[rx_payload.pipe] != NULL){
                            g_listener_callbacks[rx_payload.pipe](rx_payload.data, rx_payload.length);
                        }
                        
                    }
                }
            }
            nrf_esb_flush_rx();
            break;
    }
}

static int8_t esb_reinit(nrf_esb_mode_t esb_mode)
{
    nrf_esb_stop_rx();
    nrf_esb_disable();
    g_nrf_esb_config.mode = esb_mode;
    nrf_esb_init(&g_nrf_esb_config);

    /* set pipeline addresses again because they are reset in nrf_esb_init() */
    uint8_t addr_prefix[2] = {g_pipe_addr[ESB_PIPE_0][4], g_pipe_addr[ESB_PIPE_1][4]};
    if(nrf_esb_set_address_length(5) != NRF_SUCCESS){
        return (ESB_ERR_HAL);
    }

    if(nrf_esb_set_base_address_0(g_pipe_addr[ESB_PIPE_0]) != NRF_SUCCESS){
        return (ESB_ERR_HAL);
    }

    if(nrf_esb_set_base_address_1(g_pipe_addr[ESB_PIPE_1]) != NRF_SUCCESS){
        return (ESB_ERR_HAL);
    }

    if(nrf_esb_set_prefixes(addr_prefix, 2) != NRF_SUCCESS){
        return (ESB_ERR_HAL);
    }

    if(esb_mode == NRF_ESB_MODE_PTX){
        nrf_esb_start_tx();
    }else if (esb_mode == NRF_ESB_MODE_PRX){
        nrf_esb_start_rx();
    }
    return (ESB_ERR_OK);
}
    
int8_t esb_init(void)
{    
    g_nrf_esb_config.protocol                 = NRF_ESB_PROTOCOL_ESB_DPL;
    g_nrf_esb_config.mode                     = NRF_ESB_MODE_PRX;
    g_nrf_esb_config.event_handler            = nrf_esb_event_handler;
    g_nrf_esb_config.bitrate                  = NRF_ESB_BITRATE_1MBPS;
    g_nrf_esb_config.crc                      = NRF_ESB_CRC_16BIT;
    g_nrf_esb_config.tx_output_power          = NRF_ESB_TX_POWER_4DBM;
    g_nrf_esb_config.retransmit_delay         = 600;
    g_nrf_esb_config.retransmit_count         = 10;
    g_nrf_esb_config.tx_mode                  = NRF_ESB_TXMODE_AUTO;
    g_nrf_esb_config.selective_auto_ack       = true;

    if(nrf_esb_init(&g_nrf_esb_config) != NRF_SUCCESS){
        return (ESB_ERR_HAL);
    }

    if(nrf_esb_set_rf_channel(ESB_DEFAULT_CHANNEL) != NRF_SUCCESS){
        return (ESB_ERR_HAL);
    }

    g_initialized = 1;
    return (ESB_ERR_OK);
}

int8_t esb_set_pipeline_address(const esb_pipeline_t pipeline, const uint8_t addr[5])
{
    ESB_CHECK_PIPE_PARAM(pipeline);
    ESB_CHECK_NULL_PARAM(addr);

    memcpy(g_pipe_addr[pipeline], addr, 5);

    return (ESB_ERR_OK);
}

int8_t esb_start_listening(const esb_pipeline_t pipeline, esb_listener_callback_t listener_callback)
{
    ESB_CHECK_PIPE_PARAM(pipeline);
    ESB_CHECK_NULL_PARAM(listener_callback);
    
    g_listener_callbacks[pipeline] = listener_callback;

    return (esb_reinit(NRF_ESB_MODE_PRX));

}

int8_t esb_stop_listening(const esb_pipeline_t pipeline)
{
    ESB_CHECK_PIPE_PARAM(pipeline);
    
    g_listener_callbacks[pipeline] = NULL;

    if((g_listener_callbacks[ESB_PIPE_0] == NULL) && (g_listener_callbacks[ESB_PIPE_1] == NULL)){
        if(nrf_esb_stop_rx() != NRF_SUCCESS){
            return (ESB_ERR_HAL);
        }
    }

    return (ESB_ERR_OK);   
}

int8_t esb_set_rf_channel(const uint8_t channel)
{
    if(nrf_esb_set_rf_channel(channel) != NRF_SUCCESS){
        return (ESB_ERR_HAL);
    }

    return (ESB_ERR_OK);
}


int8_t esb_send_packet(const esb_pipeline_t pipeline, const uint8_t *payload, uint32_t payload_length)
{
    ESB_CHECK_PIPE_PARAM(pipeline);
    ESB_CHECK_NULL_PARAM(payload);
    
    if(g_initialized != 1){
        return (ESB_ERR_INIT);
    }
    
    while(g_tx_busy==1); /* wait until radio is ready */
    esb_reinit(NRF_ESB_MODE_PTX);
    memcpy(tx_payload.data, payload, payload_length);
    tx_payload.length = payload_length;
    tx_payload.pipe = pipeline;
    tx_payload.noack = true;
    if(nrf_esb_write_payload(&tx_payload) == NRF_SUCCESS){
        g_tx_busy = 1;  
    }else{
        return (ESB_ERR_HAL);
    }
    while(g_tx_busy==1); /* wait until radio is ready */
    return (ESB_ERR_OK);
}