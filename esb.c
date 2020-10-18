#include <string.h>
#include "nrf.h"
#include "nrf_esb.h"
#include "nrf_error.h"
#include "nrf_delay.h"

#include "esb.h"
#include "timebase.h"
#include "debug_swo.h"

#define ESB_TX_TIMEOUT_MS   100

static nrf_esb_payload_t        rx_payload;

static uint8_t rx_payload_available = 0;

static void nrf_esb_event_handler(nrf_esb_evt_t const * p_event)
{
    switch (p_event->evt_id)
    {
        case NRF_ESB_EVENT_TX_SUCCESS:
            debug_swo_printf("TX SUCCESS EVENT (%lu attempts)\n", p_event->tx_attempts);
            break;
        case NRF_ESB_EVENT_TX_FAILED:
            debug_swo_printf("TX FAILED EVENT\n");
            (void) nrf_esb_flush_tx();
            (void) nrf_esb_start_tx();
            break;
        case NRF_ESB_EVENT_RX_RECEIVED:
            debug_swo_printf("RX RECEIVED EVENT\n");
            while (nrf_esb_read_rx_payload(&rx_payload) == NRF_SUCCESS)
            {
                if (rx_payload.length > 0)
                {
                    rx_payload_available = 1;
                }
            }
            break;
    }
}



int8_t esb_init(void)
{
    uint8_t base_addr_0[4] = {0xE7, 0xE7, 0xE7, 0xE7};
    uint8_t base_addr_1[4] = {0xC2, 0xC2, 0xC2, 0xC2};
    uint8_t addr_prefix[8] = {0xE7, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8 };

    nrf_esb_config_t nrf_esb_config         = NRF_ESB_DEFAULT_CONFIG;
    nrf_esb_config.tx_output_power          = NRF_ESB_TX_POWER_4DBM;
    nrf_esb_config.protocol                 = NRF_ESB_PROTOCOL_ESB_DPL;
    nrf_esb_config.retransmit_delay         = 600;
    nrf_esb_config.retransmit_count         = 10;
    nrf_esb_config.bitrate                  = NRF_ESB_BITRATE_1MBPS;
    nrf_esb_config.event_handler            = nrf_esb_event_handler;
    nrf_esb_config.mode                     = NRF_ESB_MODE_PTX;
    nrf_esb_config.selective_auto_ack       = false;

    if(nrf_esb_init(&nrf_esb_config) != NRF_SUCCESS){
        return (ESB_ERR_HAL);
    }

    if(nrf_esb_set_rf_channel(40) != NRF_SUCCESS){
        return (ESB_ERR_HAL);
    }

    if(nrf_esb_set_address_length(5) != NRF_SUCCESS){
        return (ESB_ERR_HAL);
    }

    if(nrf_esb_set_base_address_0(base_addr_0) != NRF_SUCCESS){
        return (ESB_ERR_HAL);
    }

    if(nrf_esb_set_base_address_1(base_addr_1) != NRF_SUCCESS){
        return (ESB_ERR_HAL);
    }

    if(nrf_esb_set_prefixes(addr_prefix, 1) != NRF_SUCCESS){
        return (ESB_ERR_HAL);
    }

    return (0);
}

int8_t esb_set_tx_address(const uint8_t tx_addr[5])
{
    if(nrf_esb_set_base_address_0(tx_addr) != NRF_SUCCESS){
        return (ESB_ERR_HAL);
    }
    if(nrf_esb_set_prefixes(&(tx_addr[4]), 1) != NRF_SUCCESS){
        return (ESB_ERR_HAL);
    }

    return (ESB_ERR_OK);
}

int8_t esb_transmit_blocking(const uint8_t *p_tx_data, uint8_t tx_len, uint8_t *p_rx_data, uint8_t *p_rx_len)
{

    if((p_tx_data == NULL) || (p_rx_data == NULL) || (p_rx_len == NULL)){
        return (ESB_ERR_PARAM);
    }

    if(tx_len > NRF_ESB_MAX_PAYLOAD_LENGTH){
        return (ESB_ERR_SIZE);
    }   
    nrf_esb_payload_t tx_payload = {
            .pipe = 0,
            .length = tx_len,
            .noack = false
    };
    memcpy(tx_payload.data, p_tx_data, tx_len);
    memset(rx_payload.data, 0, sizeof(rx_payload.data));
    
    rx_payload_available = 0;

    if (nrf_esb_write_payload(&tx_payload) != NRF_SUCCESS)
    {
        debug_swo_printf("Sending packet failed\n");
        return (ESB_ERR_HAL);
    }

    /* wait blocking for payload */
    timebase_timeout_start(ESB_TX_TIMEOUT_MS);
    while(rx_payload_available == 0){
        if(timebase_timeout_check()){
            /* timeout*/
            debug_swo_printf("Timeout waiting for answer");
            return (ESB_ERR_TIMEOUT);
        }
    }

    memcpy(p_rx_data, rx_payload.data, rx_payload.length);
    *p_rx_len = rx_payload.length;

    debug_swo_printf("RX RECEIVED PAYLOAD: len %d | data: %02X \n", rx_payload.length, rx_payload.data[0]);

    return (ESB_ERR_OK);
}