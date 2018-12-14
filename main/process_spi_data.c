#include "process_spi_data.h"
#include "spi_message.h"
#include "spi_read.h"

void vADifferentTask( void *pvParameters )
{


   // ... Rest of task code.
}
void processSpiMessageTask(void *pvParameters){
    uint32_t count = 0;
    printf("SPI Message Processor UP and RUNNING.\n");
    while(1){
        struct SPIMessage *pxRxedMessage;

        if( xSPIQueue != 0 )
        {
            // Receive a message on the created queue.  Block for 10 ticks if a
            // message is not immediately available.
            if( xQueueReceive( xSPIQueue, &( pxRxedMessage ), ( TickType_t ) 10 ) )
            {
                count++;
                // pcRxedMessage now points to the struct AMessage variable posted
                // by vATask.
                printf("Received[%u]: ", count);
                for(int i = 0; i< SPI_BUFFER_SIZE*2; i++){
                    printf("%02x ", pxRxedMessage->raw[i]);
                }
                printf("\n");
                }
        }
    }
}
