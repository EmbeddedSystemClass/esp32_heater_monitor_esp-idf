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

    bus1.init(GPIO_MOSI, GPIO_SCLK, GPIO_CS2, VSPI_HOST);
    bus2.init(GPIO_MOSI, GPIO_SCLK, GPIO_CS1, HSPI_HOST);

    while(1) {
        bus1.read(pxMessage.part_1, 8);
        bus2.read(pxMessage.part_2, 8);
        queue->Enqueue((void*) &pxMessage);
    }
}
