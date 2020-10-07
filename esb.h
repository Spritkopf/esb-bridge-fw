#ifndef ESB_H_
#define ESB_H_

#include <stdint.h>

#define ESB_ERR_OK          0
#define ESB_ERR_HAL         -1  /* error accessing ESB function */
#define ESB_ERR_SIZE        -2  /* invalid payload length */
#define ESB_ERR_PARAM       -3  /* function parameter error */
#define ESB_ERR_TIMEOUT     -4  /* timeout waiting for an answer */


/* initialize Enhanced Shockburst (ESB) communication 
 * returns 0 if OK, otherwise -1
*/
int8_t esb_init(void);

/* Set TX target address
 * returns 0 if OK, otherwise -1
*/
int8_t esb_set_tx_address(const uint8_t tx_addr[5]);



/* transmit data, wait blocking for ACK payload 
 * returns:
 *  0  - OK
 * -1  - ESB Error
 * -2  - invalid Payload length
 * -3  - Parameter Error (NULL Pointer)
 * -4  - ESB Timeout waiting for answer
 */
int8_t esb_transmit_blocking(const uint8_t *p_txdata, uint8_t tx_len, uint8_t *p_rx_data, uint8_t *p_rx_len);

#endif