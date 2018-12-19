#include "spi_slave.hpp"
#include <algorithm>
#include <iterator>
#include <cstring>

void SlaveSPI::init(uint8_t mosi, uint8_t sck, uint8_t cs, spi_host_device_t spi_device){
    esp_err_t err;

    this->bus = spi_device;
    this->buscfg.mosi_io_num = mosi;
    this->buscfg.miso_io_num = (int) NULL;
    this->buscfg.sclk_io_num = sck;

    this->slvcfg.mode = 0;
    this->slvcfg.spics_io_num = cs;
    this->slvcfg.queue_size = 2;
    this->slvcfg.flags = 0;

    int dma_channel = (int) this->bus;

    err=spi_slave_initialize(this->bus, &buscfg, &slvcfg, dma_channel);
    assert(err==ESP_OK);

    size_t buffer_size = 128; //must be big enough for both rx/tx? 64 is too small and reads out jibberish
    this->rx_buffer = (uint8_t *) heap_caps_malloc(16*sizeof(uint8_t), MALLOC_CAP_DMA | MALLOC_CAP_32BIT);
    this->transaction = new spi_slave_transaction_t{buffer_size, NULL, NULL, this->rx_buffer, NULL};
    if((int)this->rx_buffer % 4 != 0){ //check that we are word alligned
        assert(false);
    }
}

void SlaveSPI::read(uint8_t * rx_bytes, int size){
    esp_err_t err;

    err=spi_slave_transmit(this->bus, this->transaction, portMAX_DELAY);
    assert(err==ESP_OK);

    // printf("temp:  ");
    // for (int x = 0; x<8; x++){
    //     printf("%02x ", this->rx_buffer[x]);
    // } printf("\n");

    std::copy_n(this->rx_buffer, size, rx_bytes);

}

SlaveSPI::~SlaveSPI(){
    spi_slave_free(this->bus);
    free(this->rx_buffer);
}
