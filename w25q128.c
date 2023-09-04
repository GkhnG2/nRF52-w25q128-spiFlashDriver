#include "w25q128.h"
#include "nrf_delay.h"
#include <string.h>


static volatile bool spi_xfer_done;

uint8_t spi_tx_buf[SPI_BUFSIZE];
uint8_t spi_rx_buf[SPI_BUFSIZE];


void spi_event_handler(nrf_drv_spi_evt_t const * p_event, void * p_context)
{
  spi_xfer_done = true;
}


void spi_init(nrf_drv_spi_t const * const instance)
{
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;

    spi_config.ss_pin    = 27;
    spi_config.miso_pin  = 26;
    spi_config.mosi_pin  = 25;
    spi_config.sck_pin   = 24;
    spi_config.frequency = NRF_DRV_SPI_FREQ_4M;

    APP_ERROR_CHECK(nrf_drv_spi_init(instance, &spi_config, spi_event_handler, NULL));
}


ret_code_t w25q128_reset(nrf_drv_spi_t const * const instance)
{
    ret_code_t err_code;
    
    spi_tx_buf[0] = 0x66;
    spi_tx_buf[1] = 0x99;
    spi_xfer_done = false;

    err_code = nrf_drv_spi_transfer(instance, spi_tx_buf, 2, NULL, 0);

    while(spi_xfer_done == false){};

    return err_code;
}


ret_code_t w25q128_read_page(nrf_drv_spi_t const * const instance,
                             uint32_t page_address,
                             uint8_t * p_rx_buffer,
                             uint16_t rx_buffer_length)
{
    ret_code_t err_code;

    uint32_t memAddr;
    uint16_t len;
    uint8_t  cnt = 1;
    
    len = rx_buffer_length;
    if(len > 256)
        len = 256;

    if(len > 128)
        cnt = 2;

    for(int i=0; i<cnt; i++){
    
        memAddr = 256*page_address + i*128;

        spi_tx_buf[0] = 0x03;
        spi_tx_buf[1] = (memAddr >> 16) & 0xFF;     // MSB of the memory Address
        spi_tx_buf[2] = (memAddr >>  8) & 0xFF;
        spi_tx_buf[3] = (memAddr) & 0xFF;           // LSB of the memory Address
        spi_xfer_done = false;

        err_code = nrf_drv_spi_transfer(instance, spi_tx_buf, 4, spi_rx_buf, 128+4);

        while(spi_xfer_done == false){};

        memcpy(p_rx_buffer + i*128, spi_rx_buf + 4, 128);
    }

    return err_code;
}


ret_code_t w25q128_enable_write(nrf_drv_spi_t const * const instance)
{
    ret_code_t err_code;

    spi_tx_buf[0] = 0x06;  // enable Write
    spi_xfer_done = false;
    
    err_code = nrf_drv_spi_transfer(instance, spi_tx_buf, 1, NULL, 0);

    while(spi_xfer_done == false){};

    nrf_delay_ms(5);

    return err_code;
}


ret_code_t w25q128_disable_write(nrf_drv_spi_t const * const instance)
{
    ret_code_t err_code;

    spi_tx_buf[0] = 0x04;  // disable Write
    spi_xfer_done = false;
    
    err_code = nrf_drv_spi_transfer(instance, spi_tx_buf, 1, NULL, 0);

    while(spi_xfer_done == false){};

    nrf_delay_ms(5);

    return err_code;
}


ret_code_t w25q128_Erase_Sector (nrf_drv_spi_t const * const instance, uint16_t numsector)
{
    ret_code_t err_code;

    uint32_t memAddr = numsector*16*256;     // Each sector contains 16 pages * 256 bytes

    err_code = w25q128_enable_write(instance);

    spi_tx_buf[0] = 0x20;                    // Erase sector command
    spi_tx_buf[1] = (memAddr >> 16) & 0xFF;  // MSB of the memory Address
    spi_tx_buf[2] = (memAddr >>  8) & 0xFF;
    spi_tx_buf[3] = (memAddr) & 0xFF;        // LSB of the memory Address

    spi_xfer_done = false;
    err_code = nrf_drv_spi_transfer(instance, spi_tx_buf, 4, NULL, 0);
    while(spi_xfer_done == false){};

    nrf_delay_ms(450);  // 450ms delay for sector erase

    err_code = w25q128_disable_write(instance);

    return err_code;
}


ret_code_t w25q128_write_page(nrf_drv_spi_t const * const instance,
                              uint32_t page_address,
                              uint8_t const * p_tx_buffer,
                              uint16_t tx_buffer_length){
    ret_code_t err_code;

    uint32_t memAddr;
    uint16_t len;
    uint8_t  cnt = 1;

    len = tx_buffer_length;
    if(len > 256)
        len = 256;

    if(len > 128)
        cnt = 2;

    for(int i=0; i<cnt; i++){

        err_code = w25q128_enable_write(instance);    
    
        memAddr = 256*page_address + i*128;

        spi_tx_buf[0] = 0x02;  // page program
        spi_tx_buf[1] = (memAddr >> 16) & 0xFF;  // MSB of the memory Address
        spi_tx_buf[2] = (memAddr >>  8) & 0xFF;
        spi_tx_buf[3] = (memAddr) & 0xFF;        // LSB of the memory Address
    
        memcpy(&spi_tx_buf[4], p_tx_buffer + i*128, 128);
  
        spi_xfer_done = false;
        err_code = nrf_drv_spi_transfer(instance, spi_tx_buf, 128 + 4, NULL, 0);
        while(spi_xfer_done == false){};


        nrf_delay_ms(5);

        err_code = w25q128_disable_write(instance);

    }

    return err_code;
}
