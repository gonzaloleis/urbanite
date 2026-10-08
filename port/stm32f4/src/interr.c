/**
 * @file interr.c
 * @brief Interrupt service routines for the STM32F4 platform.
 * @author SDG2. Román Cárdenas (r.cardenas@upm.es) and Josué Pagán (j.pagan@upm.es)
 * @date 2025-01-01
 */
// Include HW dependencies:
#include "port_system.h"
#include "port_button.h"
#include "port_ultrasound.h"
#include "stm32f4_button.h"
#include "stm32f4_system.h"
#include "stm32f4_ultrasound.h"

// Include headers of different port elements:
//------------------------------------------------------
// INTERRUPT SERVICE ROUTINES
//------------------------------------------------------
/**
 * @brief Interrupt service routine for the System tick timer (SysTick).
 *
 * @note This ISR is called when the SysTick timer generates an interrupt.
 * The program flow jumps to this ISR and increments the tick counter by one millisecond.
 *
 * > **TO-DO alumnos:**
 * >
 * > ✅ 1. **Increment the System tick counter `msTicks` in 1 count.** To do so, use the function `port_system_get_millis()` and `port_system_set_millis()`.
 *
 * @warning **The variable `msTicks` must be declared volatile!** Just because it is modified by a call of an ISR, in order to avoid [*race conditions*](https://en.wikipedia.org/wiki/Race_condition). **Added to the definition** after *static*.
 *
 */
void SysTick_Handler(void){
    uint32_t t = port_system_get_millis();
    port_system_set_millis(t + 1);
}

/**
 * @brief Identifica el pin que ha hecho que saltara la interrupción. Después realiza la acción deseada y limpia
 * el registro de interrupción pendiente 
 * 
 * @param void
 * 
 */

void EXTI15_10_IRQHandler(void){
    
    port_system_systick_resume();
    
    if (port_button_get_pending_interrupt(PORT_PARKING_BUTTON_ID)){
        bool button_st = port_button_get_value(PORT_PARKING_BUTTON_ID);
        if(button_st){
            port_button_set_pressed(PORT_PARKING_BUTTON_ID, false);
        }
        else{
            port_button_set_pressed(PORT_PARKING_BUTTON_ID, true);
        }
        port_button_clear_pending_interrupt(PORT_PARKING_BUTTON_ID);
    }
}

/**
 * @brief Este temporizador controla la duración del trigger del sensor (TIM3). 
 * Cuando salta la interrupción significa que el tiempo de trigger acabó y que se debe poner a 0
 */

void TIM3_IRQHandler(){

    // Configuramos el registro SR y borramos el bit UIF
    TIM3->SR &= ~ TIM_SR_UIF;

    //Indicamos que ha terminado el trigger
    port_ultrasound_set_trigger_end(PORT_REAR_PARKING_SENSOR_ID, true);
}

/**
 * @brief Este temporizador controla la duración de la señael echo (TIM2), configurado como captura de entrada
 */

void TIM2_IRQHandler(){
    
    // Se reactiva el contador tras la interrupción
    port_system_systick_resume();

    //Comprobamos si está establecido el flag UIF. Si está, el ARR habrá desbordado y por ello aumentamos el número de desbordamientos
    if(TIM2->SR & TIM_SR_UIF){
        uint32_t overflowsNumber = port_ultrasound_get_echo_overflows(PORT_REAR_PARKING_SENSOR_ID) + 1;
        port_ultrasound_set_echo_overflows(PORT_REAR_PARKING_SENSOR_ID,overflowsNumber);
        TIM2->SR &= ~TIM_SR_UIF;
    }

    //Comprobamos si está establecido el flag CC2IF. Si está, significa que ha ocurrido el evento de captura de entrada
    if(TIM2->SR & TIM_SR_CC2IF){
        // Leer el valor del CCR2
        uint32_t currentTick = TIM2->CCR2;
        if(port_ultrasound_get_echo_init_tick(PORT_REAR_PARKING_SENSOR_ID) == 0 && port_ultrasound_get_echo_end_tick(PORT_REAR_PARKING_SENSOR_ID) == 0){
            port_ultrasound_set_echo_init_tick(PORT_REAR_PARKING_SENSOR_ID,currentTick);
        } else{
            port_ultrasound_set_echo_end_tick(PORT_REAR_PARKING_SENSOR_ID, currentTick);
            port_ultrasound_set_echo_received(PORT_REAR_PARKING_SENSOR_ID, true);    
        }
    }
}

/**
 * @brief Este temporizador controla la duración de la señael echo (TIM2), configurado como captura de entrada
 */

void TIM5_IRQHandler(){

    // Configuramos el registro SR y borramos el bit UIF
    TIM5->SR &= ~ TIM_SR_UIF;

    // Indicamos que se puede realizar una nueva medición
    port_ultrasound_set_trigger_ready(PORT_REAR_PARKING_SENSOR_ID, true);

}