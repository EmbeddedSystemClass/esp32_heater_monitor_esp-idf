/* SPI Slave example, receiver (uses SPI Slave driver to communicate with sender)

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/igmp.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "soc/rtc_cntl_reg.h"
#include "rom/cache.h"
#include "esp_log.h"
#include "esp_spi_flash.h"

//tasks
#include "spi_read.h"
#include "process_spi_data.h"
#include "state_manager.h"
#include "power_status.h"

#include "spi_message.h"
#include "state_struct.h"
#include "queue_struct.h"

#include "pin_definitions.h"
#include "driver/gpio.h"
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<UP_KEY) | (1ULL<<DOWN_KEY) | (1ULL<<MODE_KEY) | (1ULL<<POWER_KEY))
#define GPIO_INPUT_PIN_SEL  ((1ULL<<POWER_LED_1) | (1ULL<<POWER_LED_2))


extern "C" {
	void app_main(void);
}

void app_main(void){

        queues_t * queues = new queues_t; // need a ptr to pass to the tasks for intra-task communication
        queues->spi_messages = new Queue( 10, sizeof( SPIMessage));
        queues->current_state = new Queue( 10, sizeof( heater_state_t));

        gpio_config_t * io_conf = new gpio_config_t();

        //disable interrupt
        io_conf->intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE;
        //set as output mode
        io_conf->mode = GPIO_MODE_OUTPUT;
        //bit mask of the pins that you want to set
        io_conf->pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
        io_conf->pull_up_en = (gpio_pullup_t) 0;
        io_conf->pull_down_en = (gpio_pulldown_t) 0;

        //configure GPIO with the given settings
        gpio_config(io_conf);

        io_conf->mode = GPIO_MODE_INPUT;
        io_conf->pin_bit_mask = GPIO_INPUT_PIN_SEL;

        gpio_config(io_conf);

        gpio_set_level((gpio_num_t)POWER_KEY, 1);
        gpio_set_level((gpio_num_t)UP_KEY, 1);
        gpio_set_level((gpio_num_t)DOWN_KEY, 1);
        gpio_set_level((gpio_num_t)MODE_KEY, 1);

        xTaskCreatePinnedToCore(
                    &powerStatus,   /* Function to implement the task */
                    "Power Status Reader", /* Name of the task */
                    2048,      /* Stack size in words */
                    queues,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    tskNO_AFFINITY);  /* Core where the task should run */

        xTaskCreatePinnedToCore(
                    &spi_read,   /* Function to implement the task */
                    "SPI Reader", /* Name of the task */
                    2048,      /* Stack size in words */
                    queues,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    tskNO_AFFINITY);  /* Core where the task should run */

        xTaskCreatePinnedToCore(
                    &processSpiMessageTask,   /* Function to implement the task */
                    "SPI Parse", /* Name of the task */
                    2048,      /* Stack size in words */
                    queues,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    tskNO_AFFINITY);  /* Core where the task should run */

        xTaskCreatePinnedToCore(
                    &manageState,   /* Function to implement the task */
                    "State Manager", /* Name of the task */
                    2048,      /* Stack size in words */
                    queues,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    tskNO_AFFINITY);  /* Core where the task should run */
 }