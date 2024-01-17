
#include "persistent_mem.h"

#include "sdk_common.h"
#include "nrf.h"
#include "nrf_fstorage.h"
#include "nrf_fstorage_nvmc.h"

static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt);

/* memory region is defined in linker file (region APP_DATA, section .app_data) */
extern uint32_t __start_app_data;
extern uint32_t __end_app_data;

NRF_FSTORAGE_DEF(nrf_fstorage_t fstorage) =
{
    /* Set a handler for fstorage events. */
    .evt_handler = fstorage_evt_handler,

    .start_addr = (uint32_t)&__start_app_data,
    .end_addr   = (uint32_t)&__end_app_data,
};


void wait_for_flash_ready(void)
{
    /* While fstorage is busy, sleep and wait for an event. */
    while (nrf_fstorage_is_busy(&fstorage))
    {
        __WFE();
    }
}


int8_t persistent_mem_init(void)
{
    ret_code_t rc;

    nrf_fstorage_api_t * p_fs_api;

    p_fs_api = &nrf_fstorage_nvmc;

    rc = nrf_fstorage_init(&fstorage, p_fs_api, NULL);

    if (rc != NRF_SUCCESS){
        return (-1);
    }
    return (0);
}

int8_t persistent_mem_read(uint32_t addr_offset, uint32_t len, void* p_dest)
{
    ret_code_t rc = nrf_fstorage_read(&fstorage, fstorage.start_addr+addr_offset, p_dest, len);
    if (rc != NRF_SUCCESS)
    {
        return (-1);
    }
    wait_for_flash_ready();
    return (0);
}

int8_t persistent_mem_write(uint32_t addr_offset, uint32_t len, void* p_src)
{
    ret_code_t rc = nrf_fstorage_write(&fstorage, fstorage.start_addr+addr_offset, p_src, len, NULL);
    if (rc != NRF_SUCCESS)
    {
        return (-1);
    }
    wait_for_flash_ready();
    return (0);
}

int8_t persistent_mem_erase(uint32_t addr_offset, uint32_t pages_cnt)
{
    ret_code_t rc = nrf_fstorage_erase(&fstorage, fstorage.start_addr+addr_offset, pages_cnt, NULL);
    if (rc != NRF_SUCCESS)
    {
        return (-1);
    }
    wait_for_flash_ready();
    return (0);
}

static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt)
{
    if (p_evt->result != NRF_SUCCESS)
    {
        return;
    }

    switch (p_evt->id)
    {
        case NRF_FSTORAGE_EVT_WRITE_RESULT:
        {
        } break;

        case NRF_FSTORAGE_EVT_ERASE_RESULT:
        {
        } break;

        default:
            break;
    }
}