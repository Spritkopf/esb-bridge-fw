#ifndef ESB_H_
#define ESB_H_

#include <stdint.h>

#define ESB_ERR_OK          0
#define ESB_ERR_INIT        -1  /* ESB module not initialized */
#define ESB_ERR_HAL         -2  /* error accessing ESB function */
#define ESB_ERR_SIZE        -3  /* invalid payload length */
#define ESB_ERR_PARAM       -4  /* function parameter error */
#define ESB_ERR_TIMEOUT     -5  /* timeout waiting for an answer */


typedef void(*esb_listener_callback_t)(uint8_t *payload, uint8_t payload_length);



/* initialize Enhanced Shockburst (ESB) communication 
 * listening_addr: Pipeline address for RX mode
 * listener_callback: gets called on incoming package (optional, pass NULL to disable)
 * returns:
 * ESB_ERR_OK       - OK
 * ESB_ERR_HAL      - ESB HAL Error
 * ESB_ERR_PARAM    - Parameter Error (NULL Pointer)
*/
int8_t esb_init(const uint8_t listening_addr[5], esb_listener_callback_t listener_callback);

/* Set TX target address
*/
void esb_set_tx_address(const uint8_t tx_addr[5]);


/* Set RF Channel
 * returns 0 if OK, otherwise -1
*/
int8_t esb_set_rf_channel(const uint8_t channel);

/* Send data
 * returns:
 * ESB_ERR_OK       - OK
 * ESB_ERR_INIT     - module not initialized
 * ESB_ERR_HAL      - ESB HAL Error
 * ESB_ERR_SIZE     - invalid Payload length
 * ESB_ERR_PARAM    - Parameter Error (NULL Pointer)
 * ESB_ERR_TIMEOUT  - ESB Timeout waiting for TX success
 */
int8_t nrf_esb_send(const uint8_t *payload, uint32_t payload_length);

#endif