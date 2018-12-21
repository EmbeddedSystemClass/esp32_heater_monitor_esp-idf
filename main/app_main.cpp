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

#include "spi_read.h"
#include "process_spi_data.h"

#include "spi_message.h"
#include "state_struct.h"
#include "queue_struct.h"

extern "C" {
	void app_main(void);
}

void app_main(void){

        queues_t * queues = new queues_t; // need a ptr to pass to the tasks for intra-task communication
        queues->spi_messages = new Queue( 10, sizeof( SPIMessage));
        queues->current_state = new Queue( 10, sizeof( heater_state_t));

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
 }