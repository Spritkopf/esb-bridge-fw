#ifndef ESB_H_
#define ESB_H_

#include <stdint.h>

/* initialize Enhanced Shockburst (ESB) communication 
 * returns 0 if OK, otherwise -1
*/
int8_t esb_init(void);

/* transmit data, wait blocking for ACK payload 
 * returns:
 *  0  - OK
 * -1  - ESB Error
 * -2  - invalid Payload length
 * -3  - Parameter Error (NULL Pointer)
 * -4  - ESB Timeout waiting for answer
 */
int8_t esb_transmit_blocking(uint8_t *p_txdata, uint8_t tx_len, uint8_t *p_rx_data, uint8_t *p_rx_len);

#endif