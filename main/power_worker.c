#include "pin_definitions.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

void powerStatus(void * powerPtr){
    bool currentPower = true;
    bool previousPower = true;

    gpio_pad_select_gpio(POWER_LED_1);
    gpio_pad_select_gpio(POWER_LED_2);

    gpio_set_direction(POWER_LED_1, GPIO_MODE_INPUT);
    gpio_set_direction(POWER_LED_2, GPIO_MODE_INPUT);

    gpio_set_pull_mode(POWER_LED_1, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(POWER_LED_2, GPIO_PULLUP_ONLY);

    int pin1;
    int pin2;

    while(1){
        pin1 = 0;
        pin2 = 0;

        // theres lots of noise for some reason, so lets just get a
        // bunch of readings. 300 seems fast enough and stable enough.
        for (int i = 0; i<300; i++){
            pin1+=gpio_get_level(POWER_LED_1);
            pin2+=gpio_get_level(POWER_LED_2);
        }
        if (pin1>pin2){
            currentPower = true;
        }else{
            currentPower = false;
        }
        if (currentPower!=previousPower){
            previousPower = currentPower;
            *(bool*)powerPtr = currentPower;
            printf("Power Monitor: %s\n", *(bool*)powerPtr ? "On":"Off");
        }
    }
}