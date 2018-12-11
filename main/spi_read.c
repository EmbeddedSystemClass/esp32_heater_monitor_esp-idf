#include "spi_read.h"
#include "pin_definitions.h"
#include "driver/spi_slave.h"
#include <string.h>


//Main application
void spi_read(void * pvParameter)
{
    //Called after a transaction is queued and ready for pickup by master. We use this to set the handshake line high.
    void my_post_setup_cb(spi_slave_transaction_t *trans) {
    }

    //Called after transaction is sent/received. We use this to set the handshake line low.
    void my_post_trans_cb(spi_slave_transaction_t *trans) {
    }
    esp_err_t ret;

    //Configuration for the SPI bus
    spi_bus_config_t buscfg1={
        .mosi_io_num=GPIO_MOSI,
        .miso_io_num=NULL,
        .sclk_io_num=GPIO_SCLK
    };
    spi_bus_config_t buscfg2={
        .mosi_io_num=GPIO_MOSI,
        .miso_io_num=NULL,
        .sclk_io_num=GPIO_SCLK
    };

    //Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg1={
        .mode=0,
        .spics_io_num=GPIO_CS1,
        .queue_size=16,
        .flags=0,
        .post_setup_cb=my_post_setup_cb,
        .post_trans_cb=my_post_trans_cb
    };

    spi_slave_interface_config_t slvcfg2={
        .mode=0,
        .spics_io_num=GPIO_CS2,
        .queue_size=16,
        .flags=0,
        .post_setup_cb=my_post_setup_cb,
        .post_trans_cb=my_post_trans_cb
    };

    //Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected.
    gpio_set_pull_mode(GPIO_MOSI, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_SCLK, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_CS1, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_CS2, GPIO_PULLUP_ONLY);

    //Initialize SPI slave interface
    ret=spi_slave_initialize(HSPI_HOST, &buscfg1, &slvcfg1, 1);
    assert(ret==ESP_OK);

    ret=spi_slave_initialize(VSPI_HOST, &buscfg2, &slvcfg2, 2);
    assert(ret==ESP_OK);

    uint32_t recvbuf1[SPI_BUFFER_SIZE]={};
    memset(recvbuf1, 0, SPI_BUFFER_SIZE);
    spi_slave_transaction_t t1;
    memset(&t1, 0, sizeof(t1));

    uint32_t recvbuf2[SPI_BUFFER_SIZE]={};
    memset(recvbuf2, 0, SPI_BUFFER_SIZE);
    spi_slave_transaction_t t2;
    memset(&t2, 0, sizeof(t2));

    while(1) {
        //Clear receive buffer, set send buffer to something sane
        memset(recvbuf1, 0, SPI_BUFFER_SIZE);
        memset(recvbuf2, 0, SPI_BUFFER_SIZE);

        //Set up a transaction of 128 bytes to send/receive
        t1.length=SPI_BUFFER_SIZE << 3;
        t1.tx_buffer=NULL;
        t1.rx_buffer=recvbuf1;

        t2.length=SPI_BUFFER_SIZE << 3;
        t2.tx_buffer=NULL;
        t2.rx_buffer=recvbuf2;
        /* This call enables the SPI slave interface to send/receive to the sendbuf and recvbuf1. The transaction is
        initialized by the SPI master, however, so it will not actually happen until the master starts a hardware transaction
        by pulling CS low and pulsing the clock etc. In this specific example, we use the handshake line, pulled up by the
        .post_setup_cb callback that is called as soon as a transaction is ready, to let the master know it is free to transfer
        data.
        */
        ret=spi_slave_transmit(HSPI_HOST, &t1, portMAX_DELAY);
        assert(ret==ESP_OK);
        ret=spi_slave_transmit(VSPI_HOST, &t2, portMAX_DELAY);
        assert(ret==ESP_OK);

        //spi_slave_transmit does not return until the master has done a transmission, so by here we have sent our data and
        //received data from the master. Print it.
        printf("Received: %08x %08x\n", *recvbuf1, *recvbuf2);
    }

}
