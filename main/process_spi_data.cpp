#include "process_spi_data.h"

#include "spi_message.h"
#include "queue.hpp"
using namespace cpp_freertos;
#include <vector>


// uint8_t spi_packet[16];
// memset(spi_packet, 0, 16);


// bool is_changed(uint8_t array1[], uint8_t array2[]){
//     for (int i = 0; i<8; i++){
//         if(spi_packet[i] != array1[i] && spi_packet[i+8] != array2[i]){
//             return true;
//         }
//     }
//     return false;
// }

// void combine(uint8_t array1[], uint8_t array2[]){
//     for(int i = 0; i<8; i++){
//         spi_packet[i] = array1[i];
//         spi_packet[i+8] = array2[i];
//     }
// }

void processSpiMessageTask(void * queuePtr){
    Queue * queue = (Queue *) queuePtr; //casts the void* mess to a queue since we pass it through
    uint32_t count = 0;
    printf("SPI Message Processor UP and RUNNING.\n");
    struct SPIMessage * pxRxedMessage = new SPIMessage();
    while(1){

        if( queue->NumItems() > 0 )
        {
            // Receive a message on the created queue.  Block for 10 ticks if a
            // message is not immediately available.
            if( queue->Dequeue((SPIMessage*) pxRxedMessage, (TickType_t) 10))
            {

                count++;
                // pcRxedMessage now points to the struct AMessage variable posted
                // by vATask.
                printf("Received[%u]: ", count);
                for(int i = 0; i< 8; i++){
                    printf("%02x ", pxRxedMessage->part_1[i]);
                }
                for(int i = 0; i< 8; i++){
                    printf("%02x ", pxRxedMessage->part_2[i]);
                }
                printf("\n");
            }
        }
    }
}
