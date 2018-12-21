#include <stdint.h>

typedef struct heater_state_t {
    uint8_t thermostat_setpoint;
    bool set_hold_mode;
    bool hold_mode;
} heater_state_t;