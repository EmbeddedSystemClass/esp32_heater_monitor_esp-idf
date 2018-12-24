#include <stdint.h>

typedef struct heater_state_t {
    uint8_t thermostat_setpoint;
    bool set_hold_mode;
    bool hold_mode;
    bool idle;
    bool power; // currently only used for desired state
} heater_state_t;