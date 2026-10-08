/**
 * @file stm32f4_button.c
 * @brief Portable functions to interact with the button FSM library. All portable functions must be implemented in this file.
 * @author Nicolás Henández Martín
 * @author Gonzalo Leis Varela
 * @date 17/03/2025
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */

/* HW dependent includes */
#include <stdio.h>
#include "port_button.h"
#include "port_system.h"
#include "stm32f4_system.h"
#include "stm32f4_button.h"


 // Used to get general information about the buttons (ID, etc.)
 // Used to get the system tick

/* Microcontroller dependent includes */
// TO-DO alumnos: include the necessary files to interact with the GPIOs


/* Typedefs --------------------------------------------------------------------*/
typedef struct
{
    GPIO_TypeDef *p_port;
    uint8_t pin;
    uint8_t pupd_mode;
    bool flag_pressed;
    
} stm32f4_button_hw_t;

/* Global variables ------------------------------------------------------------*/
static stm32f4_button_hw_t buttons_arr[] = {
    [PORT_PARKING_BUTTON_ID] = {.p_port = STM32F4_PARKING_BUTTON_GPIO, .pin = STM32F4_PARKING_BUTTON_PIN, .pupd_mode =STM32F4_GPIO_PUPDR_NOPULL},
};
/* Private functions ----------------------------------------------------------*/

/**
 * @brief Get the button status struct with the given ID.
 *
 * @param button_id Button ID.
 *
 * @return Pointer to the button state struct.
 * @return NULL If the button ID is not valid.
 */
stm32f4_button_hw_t *_stm32f4_button_get(uint32_t button_id)
{
    // Return the pointer to the button with the given ID. If the ID is not valid, return NULL.
    if (button_id < sizeof(buttons_arr) / sizeof(buttons_arr[0]))
    {
        return & buttons_arr[button_id];
    }
    else
    {
        return NULL;
    }
}

/* Public functions -----------------------------------------------------------*/

/**
 * @brief Configura las especificaciones HW del botón
 * 
 * @param button_id Índice usado para seleccionar el elemento de buttons_array[]
 */

void port_button_init(uint32_t button_id)
{
    // Retrieve the button struct using the private function and the button ID
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);

    stm32f4_system_gpio_config(p_button->p_port, p_button->pin, STM32F4_GPIO_MODE_IN, STM32F4_GPIO_PUPDR_NOPULL);
    stm32f4_system_gpio_config_exti(p_button->p_port, p_button->pin, STM32F4_TRIGGER_BOTH_EDGE|STM32F4_TRIGGER_ENABLE_INTERR_REQ);
    stm32f4_system_gpio_exti_enable(p_button->pin, 1, 0);

}

/**
 * @brief Función auxiliar que cambia el GPIO y el pin de un botón. Usada principalmente para testeo   
 * 
 * @param button_id ID del botón a cambiar 
 * @param p_port El nuevo puerto GPIO del botón
 * @param pin El nuevo pin GPIO del botón
 */

void stm32f4_button_set_new_gpio(uint32_t button_id, GPIO_TypeDef *p_port, uint8_t pin)
{
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);
    p_button->p_port = p_port;
    p_button->pin = pin;
}

/**
 * @brief Borra la interrupción pendiente del botón. Llamada desde el ISR
 * 
 * @param button_id Se usa para seleccionar correctamente la estructura del botón extraída de buttons_array[]
 */

void port_button_clear_pending_interrupt(uint32_t button_id){  
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);
    uint32_t pin = p_button -> pin;
    EXTI -> PR = (1<<pin);
}

/**
 * @brief Desactiva las interrupciones del botón. Usada para evitar interrupciones indeseadas 
 * 
 * @param button_id Se usa para seleccionar correctamente la estructura del botón extraída de buttons_array[]
 */

void port_button_disable_interrupts(uint32_t button_id){
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);
    uint32_t pin = p_button -> pin;
    stm32f4_system_gpio_exti_disable(pin);
}

/**
 * @brief Comprueba si la interrupción del botón está pendiente o no
 * 
 * @param button_id Se usa para seleccionar correctamente la estructura del botón extraída de buttons_array[]
 * 
 * @return TRUE, Sí está pendiente
 * @return FALSE, No está pendiente
 */

bool port_button_get_pending_interrupt(uint32_t button_id){
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);
    uint32_t pin = p_button -> pin;
    if(EXTI -> PR & (1<<pin)){
        return true;
    }
    return false;
}

/**
 * @brief Inidica si el botón ha sido presionado o no
 * 
 * @param button_id Se usa para seleccionar correctamente la estructura del botón extraída de buttons_array[]
 * 
 * @return TRUE, Sí ha presionado
 * @return FALSE, No ha presionado
 */

bool port_button_get_pressed(uint32_t button_id){
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);
    return p_button -> flag_pressed;
}

/**
 * @brief Obtiene el valor de la GPIO conectada al botón, osea, indica si el botón está siendo presionado
 * 
 * @param button_id Índice usado para seleccionar el elemento de buttons_array[]
 * 
 * @return TRUE, Sí está siendo presionado 
 * @return FALSE, No está siendo presionado
 */

bool port_button_get_value(uint32_t button_id){
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);
    return stm32f4_system_gpio_read(p_button -> p_port, p_button -> pin);
}

/**
 * @brief Establece el estado de del botón
 * 
 * @param button_id Se usa para seleccionar correctamente la estructura del botón extraída de buttons_array[]
 * @param pressed Estado del botón
 * 
 * @return TRUE, Sí está siendo presionado 
 * @return FALSE, No está siendo presionado
 */

void port_button_set_pressed(uint32_t button_id, bool pressed){
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);
    p_button -> flag_pressed = pressed; 
}