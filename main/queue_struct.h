#include "queue.hpp"
using namespace cpp_freertos;

typedef struct queues_t {
    Queue * spi_messages;
    Queue * current_state;
    Queue * desired_state;
} queues_t;