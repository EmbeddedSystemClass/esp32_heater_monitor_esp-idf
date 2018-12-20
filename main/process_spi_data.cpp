#include "process_spi_data.h"

#include "spi_message.h"
#include "queue.hpp"
using namespace cpp_freertos;
#include <algorithm>
#include <vector>
#include <iterator>

using namespace std;

vector<uint8_t> current;
vector<uint8_t> last;

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
                current.clear();
                copy(&pxRxedMessage->part_1[0], &pxRxedMessage->part_1[7], back_inserter(current));
                copy(&pxRxedMessage->part_2[0], &pxRxedMessage->part_2[7], back_inserter(current));


                if(current != last){
                    last.clear();
                    count++;
                    last = current;

                    printf("Received[%u]: ", count);
                    for(auto& element : last) {
                        printf("0x%02x ", element);
                    }
                    printf("\n");
                }

                // pcRxedMessage now points to the struct AMessage variable posted
                // by vATask.

            }
        }
    }
}
