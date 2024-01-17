#ifndef _PERSISTENT_MEM_H
#define _PERSISTENT_MEM_H

/* module for storing data in FLASH */

#include <stdint.h>

int8_t persistent_mem_init(void);

int8_t persistent_mem_read(uint32_t addr_offset, uint32_t len, void* p_dest);
int8_t persistent_mem_write(uint32_t addr_offset, uint32_t len, void* p_src);
int8_t persistent_mem_erase(uint32_t addr_offset, uint32_t pages_cnt);


#endif