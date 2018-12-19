#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "driver/spi_slave.h"
#define SPI_BUFFER_SIZE 8

class SlaveSPI {
    private:
        uint8_t * rx_buffer;
        spi_bus_config_t buscfg;
        spi_slave_interface_config_t slvcfg;
        spi_slave_transaction_t * transaction;
        spi_host_device_t bus;
    public:
        void init(uint8_t mosi, uint8_t sck, uint8_t cs, spi_host_device_t spi_device);
        void read(uint8_t[], int size);
	~SlaveSPI();
};
