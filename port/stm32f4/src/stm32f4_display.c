/**
 * @file stm32f4_display.c
 * @brief Portable functions to interact with the display system FSM library. All portable functions must be implemented in this file.
 * @author Nicolás Hernández Martín
 * @author Gonzalo Leis Varela
 * @date 08/04/2025
 */

/* Standard C includes */

/* HW dependent includes */
#include <stdio.h>
#include "port_display.h"
#include "port_system.h"
#include "stm32f4_system.h"
#include "stm32f4_display.h"
/* Microcontroller dependent includes */

/* Defines --------------------------------------------------------------------*/

/* Typedefs --------------------------------------------------------------------*/
typedef struct{

    GPIO_TypeDef * p_port_red;          //GPIO ROJO
    uint8_t pin_red;                    //PIN ROJO
    GPIO_TypeDef * p_port_green;        //GPIO VERDE
    uint8_t pin_green;                  //PIN VERDE
    GPIO_TypeDef * p_port_blue;         //GPIO AZUL
    uint8_t pin_blue;                   //PIN AZUL

} stm32f4_display_hw_t;

/* Global variables */

static stm32f4_display_hw_t displays_arr [] = {
    [PORT_REAR_PARKING_DISPLAY_ID] = {.p_port_red = STM32F4_REAR_PARKING_DISPLAY_RGB_R_GPIO, .pin_red = STM32F4_REAR_PARKING_DISPLAY_RGB_R_PIN, .p_port_green = STM32F4_REAR_PARKING_DISPLAY_RGB_G_GPIO, .pin_green = STM32F4_REAR_PARKING_DISPLAY_RGB_G_PIN, .p_port_blue = STM32F4_REAR_PARKING_DISPLAY_RGB_B_GPIO, .pin_blue = STM32F4_REAR_PARKING_DISPLAY_RGB_B_PIN},
};

/* Private functions -----------------------------------------------------------*/

/**
 * @brief Obtiene la estructura del display para el identificador dado
 *
 * @param display_id Índice usado para seleccionar un elemento de displays_arr[]
 * 
 * @return Devuelve un puntero a la estructura
 */

stm32f4_display_hw_t *_stm32f4_display_get(uint32_t display_id){
    return &displays_arr[display_id];
}

/**
 * @brief Configura el PWM que controla cada uno de los LED de un display
 *
 * @param display_id Índice usado para seleccionar un elemento de displays_arr[]
 */

void _timer_pwm_config(uint32_t display_id){

    // 1- Habilita la fuente de reloj del timer
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    // 2- Desactiva el contador y activa el ARPE
    TIM4->CR1 &= ~TIM_CR1_CEN;
    TIM4->CR1 |= TIM_CR1_ARPE;

    // 3- Reinicia el contador y establece los valores del Prescaler y el ARR
    TIM4->CNT = 0;
    TIM4->PSC = 4;
    TIM4->ARR = 63999;

    // 4- Deshabilita la comparación a la salida para cada canal 
    TIM4->CCER &= ~TIM_CCER_CC1E; 
    TIM4->CCER &= ~TIM_CCER_CC3E;
    TIM4->CCER &= ~TIM_CCER_CC4E;

    // 5- Borra los bits P y NP del registro de comparación de salida para cada canal
    TIM4->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC1NP);
    TIM4->CCER &= ~(TIM_CCER_CC3P | TIM_CCER_CC3NP);
    TIM4->CCER &= ~(TIM_CCER_CC4P | TIM_CCER_CC4NP);

    // 6- Configure ambos modos PWM en 1 y habilite la precarga para cada uno de los canales correspondientes. 
    
    TIM4->CCMR1 &= ~(TIM_CCMR1_OC1M);
    TIM4->CCMR1 |= (6 << TIM_CCMR1_OC1M_Pos);  
    TIM4->CCMR1 |= TIM_CCMR1_OC1PE; 
    
    TIM4->CCMR2 &= ~(TIM_CCMR2_OC3M);
    TIM4->CCMR2 |= (6 << TIM_CCMR2_OC3M_Pos);  
    TIM4->CCMR2 |= TIM_CCMR2_OC3PE;   
    
    TIM4->CCMR2 &= ~(TIM_CCMR2_OC4M);
    TIM4->CCMR2 |= (6 << TIM_CCMR2_OC4M_Pos);  
    TIM4->CCMR2 |= TIM_CCMR2_OC4PE;

    // 7- Genere un evento de actualización configurando el bit UG. Esto cargará los valores de los registros ARR y PSC en los registros activos.
    TIM4->EGR |= TIM_EGR_UG;

}

/**
 * @brief Establece los valores del registro de captura/comparación para cada canal del LED RGB asignado a un color.
 * Esta función desactiva el temporizador asociado a los LED RGB, establece los valores del registro de captura/comparación para cada canal del LED RGB y lo activa.
 *
 * @param display_id Índice usado para seleccionar un elemento de displays_arr[]
 * @param color Color a establecer
 */

void port_display_set_rgb(uint32_t display_id, rgb_color_t color){

    if((display_id == PORT_REAR_PARKING_DISPLAY_ID)){

        // Desactivación del contador del TIM4
        TIM4->CR1 &= ~TIM_CR1_CEN;
        
        if(color.r == 0 && color.g == 0 && color.b == 0){
            TIM4->CCER &= ~TIM_CCER_CC1E;
            TIM4->CCER &= ~TIM_CCER_CC3E;
            TIM4->CCER &= ~TIM_CCER_CC4E;
            return;
        }

        if(color.r == 0){
            TIM4->CCER &= ~TIM_CCER_CC1E;
        } else{
            uint32_t red = ((color.r*(TIM4->ARR + 1))/255) - 1;
            TIM4->CCR1 = red;
            TIM4->CCER |= TIM_CCER_CC1E;
        }

        if(color.g == 0){
            TIM4->CCER &= ~TIM_CCER_CC3E;
        } else{
            uint32_t green = ((color.g*(TIM4->ARR + 1))/255) - 1;
            TIM4->CCR3 = green;
            TIM4->CCER |= TIM_CCER_CC3E;
        }

        if(color.b == 0){
            TIM4->CCER &= ~TIM_CCER_CC4E;
        } else{
            uint32_t blue = ((color.b*(TIM4->ARR + 1))/255) - 1;
            TIM4->CCR4 = blue;
            TIM4->CCER |= TIM_CCER_CC4E;
        }

        TIM4->EGR |= TIM_EGR_UG;
        TIM4->CR1 |= TIM_CR1_CEN;

        return;

    }

    return;

}

/**
 * @brief Configura las especificaciones Hardware de un display pasado como parámetro
 *
 * @param display_id Índice usado para seleccionar un elemento de displays_arr[]
 */

void port_display_init(uint32_t display_id){
    
    // 1- Obtenemos la estructura a partir del ID
    stm32f4_display_hw_t *display = _stm32f4_display_get(display_id);
    
    // 2- Configuramos el LED en modo alterno sin pull-up ni pull-down
    stm32f4_system_gpio_config(display->p_port_red, display->pin_red, STM32F4_GPIO_MODE_AF, STM32F4_GPIO_PUPDR_NOPULL);
    stm32f4_system_gpio_config(display->p_port_green, display->pin_green, STM32F4_GPIO_MODE_AF, STM32F4_GPIO_PUPDR_NOPULL);
    stm32f4_system_gpio_config(display->p_port_blue, display->pin_blue, STM32F4_GPIO_MODE_AF, STM32F4_GPIO_PUPDR_NOPULL);

    // 3- Configuramos la función alterna del LED
    stm32f4_system_gpio_config_alternate(display->p_port_red, display->pin_red, STM32F4_AF2);
    stm32f4_system_gpio_config_alternate(display->p_port_green, display->pin_green, STM32F4_AF2);
    stm32f4_system_gpio_config_alternate(display->p_port_blue, display->pin_blue, STM32F4_AF2);

    // 4- Configuramos el timer y el PWM
    _timer_pwm_config(display_id);

    // 5- Apagamos el LED
    port_display_set_rgb(display_id, COLOR_OFF);

}	

/* Public functions -----------------------------------------------------------*/
