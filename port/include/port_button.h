/**
 * @file port_button.h
 * @brief Header for the portable functions to interact with the HW of the buttons. The functions must be implemented in the platform-specific code.
 * @author Nicolás Henández Martín
 * @author Gonzalo Leis Varela
 * @date 17/03/2025
 */

 #ifndef PORT_BUTTON_H_
 #define PORT_BUTTON_H_
 #define PORT_PARKING_BUTTON_ID 0 
 #define PORT_PARKING_BUTTON_DEBOUNCE_TIME_MS 200
 
 /* Includes ------------------------------------------------------------------*/
 /* Standard C includes */
 #include <stdint.h>
 #include <stdbool.h>
 
 /* Defines and enums ----------------------------------------------------------*/
 /* Defines */
 // Define here all the button identifiers that are used in the system
 
 /* Function prototypes and explanation -------------------------------------------------*/


/**
 * @brief Configura las especificaciones HW de un botón
 *
 * @param button_id Índice usado para seleccionar un elemento de buttons_arr[]
 */

void port_button_init (uint32_t button_id);

/**
 * @brief Indica si el botón a sido pulsado o no
 *
 * @param button_id Índice usado para seleccionar un elemento de buttons_arr[]
 * 
 * @return true: el botón ha sido pulsado
 * @return false: no ha sido pulsado
 */

bool port_button_get_pressed (uint32_t button_id);

/**
 * @brief Obtiene el valor de la GPIO conectada al botón
 *
 * @param button_id Índice usado para seleccionar un elemento de buttons_arr[]
 * 
 * @return booleano
 */

bool port_button_get_value (uint32_t button_id);

/**
 * @brief Establece la el estado de pulsación del botón
 *
 * @param button_id Índice usado para seleccionar un elemento de buttons_arr[]
 * @param pressed Booleano que establece el estado de pulsación
 */

void port_button_set_pressed (uint32_t button_id, bool pressed);

/**
 * @brief Obtiene el estado de la interrupción
 *
 * @param button_id Índice usado para seleccionar un elemento de buttons_arr[]
 * 
 * @return true: el botón ha sido pulsado
 * @return false: no ha sido pulsado
 */

bool port_button_get_pending_interrupt (uint32_t button_id);

/**
 * @brief Borra la interrupción pendiente
 *
 * @param button_id Índice usado para seleccionar un elemento de buttons_arr[]
 */

void port_button_clear_pending_interrupt (uint32_t button_id);

/**
 * @brief Desactiva las interrupciones del botón
 *
 * @param button_id Índice usado para seleccionar un elemento de buttons_arr[]
 */

void port_button_disable_interrupts (uint32_t button_id);
 
 #endif