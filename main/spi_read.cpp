#include "spi_read.h"
#include "pin_definitions.h"
#include <string.h>
#include <iostream>
#include "freertos/task.h"
#include "spi_slave.hpp"

#include "spi_message.h"
#include "queue.hpp"
using namespace cpp_freertos;

SlaveSPI bus1;
SlaveSPI bus2;

//Main application
void spi_read(void * pvParameter)
{
    Queue * queue = (Queue *) pvParameter;
    SPIMessage pxMessage;
    //uint8_t mosi, uint8_t sck, uint8_t cs, spi_host_device_t spi_device

    bus1.init(GPIO_MOSI, GPIO_SCLK, GPIO_CS2, VSPI_HOST);
    bus2.init(GPIO_MOSI, GPIO_SCLK, GPIO_CS1, HSPI_HOST);

    while(1) {
        bus1.read(pxMessage.part_1, 8);
        bus2.read(pxMessage.part_2, 8);

        // printf("msg1: ");
        // for (int x = 0; x<8; x++){
        //     printf("0x%02x (0x%02x)  ", pxMessage.part_1[x], pxMessage.part_2[x]);
        // }printf("\n");

        queue->Enqueue((void*) &pxMessage);
    }
}
