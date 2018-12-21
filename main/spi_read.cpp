#include "spi_read.h"
#include "pin_definitions.h"
#include "spi_slave.hpp"
#include "spi_message.h"
#include "queue_struct.h"

SlaveSPI bus1;
SlaveSPI bus2;
SPIMessage pxMessage;

//Main application
void spi_read(void * queuePtr)
{
    queues_t * queues = (queues_t*) queuePtr;

    Queue * spi_message_queue = queues->spi_messages;

    bus1.init(GPIO_MOSI, GPIO_SCLK, GPIO_CS2, VSPI_HOST);
    bus2.init(GPIO_MOSI, GPIO_SCLK, GPIO_CS1, HSPI_HOST);

    while(1) {

        bus1.read(pxMessage.part_1, 8);
        bus2.read(pxMessage.part_2, 8);
        spi_message_queue->Enqueue((void*) &pxMessage);
    }
}
