#include "process_spi_data.h"
#include "spi_message_structs.h"

#include "queue_struct.h"

#include "spi_message.h"

#include <algorithm>
#include <vector>
#include <deque>
#include <iterator>

using namespace std;

vector<uint8_t> current;
vector<uint8_t> last;

/*
* given some message, set value if a legal value is displayed.
* returns validity boolean
*/
bool digit_lookup(uint8_t message[16], uint8_t & value){
    bool lcd_value_valid = false;
    uint8_t _value = 0;

    uint8_t raw_tens = message[11]&0xfe;

    auto tens = spi_to_digit.find(raw_tens);
    if(tens != spi_to_digit.end()){
        _value += 10 * tens->second;
    }

    uint8_t raw_ones = message[13]&0xfe;

    auto ones = spi_to_digit.find(raw_ones);
    if(ones != spi_to_digit.end()){
        _value += ones->second;
        value = _value;
        lcd_value_valid = true;
    }

    return lcd_value_valid;
}

void processSpiMessageTask(void * queuePtr){
    struct queues_t * queues = (queues_t*) queuePtr;//casts the void* mess to a queue since we pass it through

    Queue * spi_message_queue = queues->spi_messages;
    uint32_t count = 0;
    printf("SPI Message Processor UP and RUNNING.\n");
    struct SPIMessage * pxRxedMessage = new SPIMessage();

    uint8_t lcd_digits;

    deque<bool> hold_values;

    while(hold_values.size()<5){ //assume not in hold mode
        hold_values.push_back(false);
    }

    bool idle_mode = false;
    bool hold_mode = false;
    bool hold_set_mode = false;
    uint8_t thermostat_temperature = 0;
    uint8_t hold_set_mode_count=0;

    while(1){


        if( spi_message_queue->NumItems() > 0 )
        {
            // Receive a message on the created queue.  Block for 10 ticks if a
            // message is not immediately available.
            if( spi_message_queue->Dequeue((SPIMessage*) pxRxedMessage, (TickType_t) 10))
            {
                current.clear();
                copy(&pxRxedMessage->part_1[0], &pxRxedMessage->part_1[8], back_inserter(current));
                copy(&pxRxedMessage->part_2[0], &pxRxedMessage->part_2[8], back_inserter(current));

                idle_mode = equal(current.begin(), current.end(), idle);

                //BEGIN detect hold set mode
                hold_values.pop_back(); // remove oldest hold value
                hold_values.push_front(hold(current.data()[11])); // add current hold value

                uint8_t is_hold = 0;
                uint8_t not_hold = 0;

                // count hold_values. if 3 true and 2 false, set hold_set_mode true, else false
                for(auto x: hold_values){
                    if(x){
                        is_hold++;
                    } else {
                        not_hold++;
                    }
                }

                if((is_hold==2 && not_hold==3)||(is_hold==3 && not_hold==2)){ //sometimes the on/off ratio flips
                    hold_set_mode_count++;
                    if(hold_set_mode_count>2){ // buffer out mode change false positives by requiring it to occur three times
                         hold_set_mode = true;
                    }
                } else {
                    hold_set_mode_count=0; //reset counter
                    hold_set_mode = false;
                }
                //END hold set mode detection

                if(!idle_mode){
                    if(is_hold>0){ // hold persists through idle state, so lets only evaluate it when not idle
                        hold_mode=true;
                    } else {
                        hold_mode=false;
                    }

                    // lets set hold mode, and the target temp.
                    if(current != last){
                        //save off stable message
                        last.clear();
                        last = current;

                        //if we have some lcd value to report, lets fetch that and print it.

                        if(digit_lookup(current.data(), lcd_digits)){
                            printf("LCD Digits: %d ", lcd_digits);
                            if (hold_set_mode){ // when we are in hold-set mode, the thermostat's temp is displayed
                                thermostat_temperature = lcd_digits;
                            }
                        }

                        printf("ThermostatTemp: %d HoldSet: %s Hold: %s ", thermostat_temperature, hold_set_mode ? "true":"false", hold_mode ? "true":"false");

                        // count++;

                        // printf("Received[%u]: ", count);
                        // for(auto& element : last) {
                        //     printf("0x%02x ", element);
                        // }
                        printf("\n");
                    }
                } else {
                    hold_set_mode = false;
                }
            }
        }
    }
}
