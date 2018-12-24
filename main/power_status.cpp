#include "pin_definitions.h"
#include "driver/gpio.h"
#include "queue_struct.h"

#define TIME_WINDOWS 100

void powerStatus(void * queuePtr){
    struct queues_t * queues = (queues_t*) queuePtr; //casts the void* mess to a queue since we pass it through

    bool currentPower = false;
    bool previousPower = false;

    queues->power = &currentPower;

    while(1){
        int pin1 = 0;
        int pin2 = 0;

        for(int i = 0; i<TIME_WINDOWS; i++){
            pin1 += gpio_get_level( (gpio_num_t) POWER_LED_1 );
            pin2 += gpio_get_level( (gpio_num_t) POWER_LED_2 );
        }

        if(pin1>pin2){
            currentPower = true;
        } else if (pin2>pin1) {
            currentPower = false;
        }

        if (currentPower != previousPower){
            previousPower = currentPower;
        }

    }
}