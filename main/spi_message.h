#include <stdint.h>

typedef struct SPIMessage
{
    uint8_t part_1[8];
    uint8_t part_2[8];
} xSPIMessage;