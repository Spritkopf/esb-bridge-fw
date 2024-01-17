/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

__start_flash = 0x0;
__size_flash = 0x80000;
__size_app_data = 0x1000;
__start_ram = 0x20000000;
__size_ram = 0x40000;

MEMORY
{
  FLASH (rx) :  ORIGIN = __start_flash, LENGTH = __size_flash - __size_app_data
  APP_DATA(rx): ORIGIN = __start_flash + __size_flash - __size_app_data, LENGTH = __size_app_data
  RAM (rwx) :   ORIGIN = __start_ram, LENGTH = __size_ram
}

SECTIONS
{
}

SECTIONS
{
  . = ALIGN(4);
  .mem_section_dummy_ram :
  {
  }
  .log_dynamic_data :
  {
    PROVIDE(__start_log_dynamic_data = .);
    KEEP(*(SORT(.log_dynamic_data*)))
    PROVIDE(__stop_log_dynamic_data = .);
  } > RAM
  .log_filter_data :
  {
    PROVIDE(__start_log_filter_data = .);
    KEEP(*(SORT(.log_filter_data*)))
    PROVIDE(__stop_log_filter_data = .);
  } > RAM

} INSERT AFTER .data;

SECTIONS
{
  .mem_section_dummy_rom :
  {
  }
  .log_const_data :
  {
    PROVIDE(__start_log_const_data = .);
    KEEP(*(SORT(.log_const_data*)))
    PROVIDE(__stop_log_const_data = .);
  } > FLASH
  .log_backends :
  {
    PROVIDE(__start_log_backends = .);
    KEEP(*(SORT(.log_backends*)))
    PROVIDE(__stop_log_backends = .);
  } > FLASH
    .nrf_balloc :
  {
    PROVIDE(__start_nrf_balloc = .);
    KEEP(*(.nrf_balloc))
    PROVIDE(__stop_nrf_balloc = .);
  } > FLASH

} INSERT AFTER .text

SECTIONS {
  .app_data :
  {
    PROVIDE(__start_app_data = .);
    . = . + __size_app_data;
    PROVIDE(__end_app_data = .);
  } > APP_DATA
}

INCLUDE "nrf_common.ld"
