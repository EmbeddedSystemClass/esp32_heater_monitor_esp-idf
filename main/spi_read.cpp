#include "spi_read.h"
#include "pin_definitions.h"
#include "spi_slave.hpp"
#include "spi_message.h"
#include "queue.hpp"

#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

using namespace cpp_freertos;

SlaveSPI bus1;
SlaveSPI bus2;
Queue * queue;
SPIMessage pxMessage;


//Main application
void spi_read(void * pvParameter)
{
    queue = (Queue *) pvParameter;

    bus1.init(GPIO_MOSI, GPIO_SCLK, GPIO_CS2, VSPI_HOST);
    bus2.init(GPIO_MOSI, GPIO_SCLK, GPIO_CS1, HSPI_HOST);

    while(1) {
        /*
        not sure why but this keeps the watchdog timer from being triggered
        */
        TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
        TIMERG0.wdt_feed=1;
        TIMERG0.wdt_wprotect=0;

        bus1.read(pxMessage.part_1, 8);
        bus2.read(pxMessage.part_2, 8);
        queue->Enqueue((void*) &pxMessage);
    }
}
