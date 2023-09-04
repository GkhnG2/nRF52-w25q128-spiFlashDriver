#ifndef W25Q128_H
#define W25Q128_H

#include "nrf_drv_spi.h"

#define SPI_BUFSIZE 132


void spi_init(nrf_drv_spi_t const * const instance);

ret_code_t w25q128_reset(nrf_drv_spi_t const * const instance);

ret_code_t w25q128_read_page(nrf_drv_spi_t const * const instance,
                             uint32_t page_address,
                             uint8_t * p_rx_buffer,
                             uint16_t rx_buffer_length);

ret_code_t w25q128_enable_write(nrf_drv_spi_t const * const instance);

ret_code_t w25q128_disable_write(nrf_drv_spi_t const * const instance);

ret_code_t w25q128_Erase_Sector (nrf_drv_spi_t const * const instance, uint16_t numsector);

ret_code_t w25q128_write_page(nrf_drv_spi_t const * const instance,
                              uint32_t page_address,
                              uint8_t const * p_tx_buffer,
                              uint16_t tx_buffer_length);

#endif