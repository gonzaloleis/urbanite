#include <stdio.h>

#include "fsm_display.h"
#include "port_display.h"
#include "port_system.h"
#include "stm32f4_system.h"

/* Defines */
#define PORT_REAR_PARKING_DISPLAY_ID 0 /*!< Ultrasound sensor identifier @hideinitializer */

int main(void)
{
    // Initialize the system
    port_system_init();

    // Initialize the button FSM
    // Reserve space memory in the heap for the FSM
    fsm_display_t *p_fsm_display_rear = fsm_display_new(PORT_REAR_PARKING_DISPLAY_ID);

    while (1)
    {
        // In every iteration, we activate the display and fire the FSM
        fsm_display_set_status(p_fsm_display_rear, true);

        // Set a new distance to the object as the vehicle moves backwards in steps of 1 cm
        for (int16_t distance_cm = 250; distance_cm >= 0; distance_cm--)
        {
            fsm_display_set_distance(p_fsm_display_rear, distance_cm);
            fsm_display_fire(p_fsm_display_rear);
            printf("[%ld] Display at distance of %d cm\n", port_system_get_millis(), distance_cm);
            port_system_delay_ms(10);
        }
        // Stop the display to ensure that the RGB LED is turned off
        fsm_display_set_status(p_fsm_display_rear, false);
        fsm_display_fire(p_fsm_display_rear);
    }

    return 0;
}
