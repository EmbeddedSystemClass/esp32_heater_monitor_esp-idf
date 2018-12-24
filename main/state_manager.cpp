#include "state_manager.h"
#include "queue_struct.h"
#include "state_struct.h"
#include "pin_definitions.h"

#include "freertos/task.h"
#include "driver/gpio.h"

void press_button(int button){
    gpio_set_level((gpio_num_t) button, 0);
    vTaskDelay(450 / portTICK_RATE_MS); //wait 500ms
    gpio_set_level((gpio_num_t) button, 1);
    vTaskDelay(500 / portTICK_RATE_MS);
}

void set_hold_mode(bool currentPower){
    if(currentPower){ //when the power is on, turn off first to set a baseline
        printf("power evaluated true.\n");
        press_button(POWER_KEY);
        vTaskDelay(500 / portTICK_RATE_MS);
    }
    press_button(POWER_KEY);
    vTaskDelay(500 / portTICK_RATE_MS);
    press_button(MODE_KEY);
    press_button(MODE_KEY);
    press_button(MODE_KEY);
    // press_button(UP_KEY); //forces hold set mode
}

void print_state(heater_state_t * state){
    printf("Idle: %s ThermostatTemp: %d HoldSet: %s Hold: %s \n",
        state->idle ? "true": "false",
        state->thermostat_setpoint,
        state->set_hold_mode ? "true":"false",
        state->hold_mode ? "true":"false"
    );
}


void getLatestState(Queue * queue, heater_state_t * state){
    while(!queue->IsEmpty()){
        queue->Dequeue((heater_state_t*) state, 200 / portTICK_RATE_MS);
    }
    print_state(state);
}

void manageState(void * queuePtr){
    struct queues_t * queues = (queues_t*) queuePtr; //casts the void* mess to a queue since we pass it through
    Queue * state_queue = queues->current_state;
    Queue * desired_queue = queues->desired_state;

    struct heater_state_t * desiredState = new heater_state_t();
    struct heater_state_t * currentState = new heater_state_t();

    printf("State Manager UP and RUNNING.\n");

    while(1){
        bool desiredStateIsEmpty = desired_queue->IsEmpty();
        bool currentStateIsEmpty = state_queue->IsEmpty();

        if (!currentStateIsEmpty){
            getLatestState(state_queue, currentState);
        }

        if(!desiredStateIsEmpty) // we only want to manage state when theres state to manage
        {
            while(!desired_queue->IsEmpty()){
                desired_queue->Dequeue((heater_state_t*) desiredState, (TickType_t) 10);
            }

            if(!desiredState->power){
                while(* queues->power){ //power is managed externally
                    press_button(POWER_KEY);
                    getLatestState(state_queue, currentState);
                    vTaskDelay(200 / portTICK_RATE_MS);
                }
                continue; //don't apply the rest of the state...
            }

            if (currentState->set_hold_mode == false && currentState->hold_mode == true){
                //if we are in hold mode, theres a chance we are also in set mode.
                //The set bool requires 2 OR 3 of the last 5 messages on the bus contain
                //a hold flag. we will wait one second for a possible newer status
                state_queue->Dequeue((heater_state_t*) currentState, 1000 / portTICK_RATE_MS);
                print_state(currentState);
            }

            while(desiredState->hold_mode != currentState->hold_mode){
                printf("setting HOLD mode, current power: %s\n", *queues->power ? "on": "off");
                set_hold_mode(* queues->power);
                vTaskDelay(500 / portTICK_RATE_MS); //let things stabilize
                getLatestState(state_queue, currentState);
            }

            while(desiredState->thermostat_setpoint != currentState->thermostat_setpoint){
                while(currentState->idle || !currentState->hold_mode){
                    press_button(MODE_KEY); // Wake the display up
                    getLatestState(state_queue, currentState);
                }
                if(desiredState->thermostat_setpoint > currentState->thermostat_setpoint){
                    printf("pressing UP\n");
                    press_button(UP_KEY);
                }
                if(desiredState->thermostat_setpoint < currentState->thermostat_setpoint){
                    printf("pressing DOWN\n");
                    press_button(DOWN_KEY);
                }
                getLatestState(state_queue, currentState);
            }
        }
    }
};