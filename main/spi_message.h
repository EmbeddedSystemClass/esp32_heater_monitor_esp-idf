#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

struct SPIMessage
{
    uint8_t raw[16];
} xSPIMessage;

QueueHandle_t xSPIQueue;