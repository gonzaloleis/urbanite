#include <stdio.h>

#include "fsm_button.h"
#include "port_button.h"
#include "port_system.h"
#include "stm32f4_system.h"

/* Defines */
#define CHANGE_MODE_BUTTON_TIME_MS 1000  /*!< Time in ms to change mode (long press) @hideinitializer */

int main(void)
{
    // Initialize the system
     port_system_init();

    // Initialize the button FSM
    // Reserve space memory in the heap for the FSM
    fsm_button_t *p_fsm_button = fsm_button_new(PORT_PARKING_BUTTON_DEBOUNCE_TIME_MS, PORT_PARKING_BUTTON_ID);

    while (1)
    {
        // In every iteration, we fire the FSM and retrieve the duration of the button press
        fsm_button_fire(p_fsm_button);

        uint32_t duration = fsm_button_get_duration(p_fsm_button);
        if (duration > 0)
        {
            printf("Button %d pressed for %ld ms", PORT_PARKING_BUTTON_ID, duration);
            // If the button is pressed for more than CHANGE_MODE_BUTTON_TIME_MS, we toggle the LED
            if (duration >= CHANGE_MODE_BUTTON_TIME_MS)
            {
                printf(" (long press detected)");
            }
            printf("\n");
            // We always reset the duration after reading it
            fsm_button_reset_duration(p_fsm_button);
        }
    }

    return 0;
}
