#include <stdio.h>

#include "fsm_ultrasound.h"
#include "port_ultrasound.h"
#include "port_system.h"
#include "stm32f4_system.h"

/* Defines */
#define PORT_REAR_PARKING_SENSOR_ID 0 /*!< Ultrasound sensor identifier @hideinitializer */

int main(void)
{
    // Initialize the system
    port_system_init();

    // Reserve space memory in the heap for the FSM
    fsm_ultrasound_t *p_fsm_ultrasound_rear = fsm_ultrasound_new(PORT_REAR_PARKING_SENSOR_ID);

    // Request a new distance measurement and fire the FSM
    fsm_ultrasound_set_status(p_fsm_ultrasound_rear, true);

    while (1)
    {
        // Wait until the distance measurement has been completed
        while (fsm_ultrasound_get_new_measurement_ready(p_fsm_ultrasound_rear) == false)
        {
            fsm_ultrasound_fire(p_fsm_ultrasound_rear);
            port_system_delay_ms(10); // Wait to let the FSM to process the new measurement
        }

        uint32_t distance = fsm_ultrasound_get_distance(p_fsm_ultrasound_rear);
        printf("[%ld] Distance: %ld cm\n", port_system_get_millis(), distance);
    }

    return 0;
}
