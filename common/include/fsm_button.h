/**
 * @file fsm_button.h
 * @brief Header for fsm_button.c file.
 * @author Nicolás Henández Martín
 * @author Gonzalo Leis Varela
 * @date 17/03/2025
*/

#ifndef FSM_BUTTON_H_
#define FSM_BUTTON_H_
 
/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>
#include <stdbool.h>
 
/* Other includes */
#include "fsm.h"
 
/* Defines and enums ----------------------------------------------------------*/
/* Enums */
enum FSM_BUTTON {

    BUTTON_RELEASED = 0,
    BUTTON_RELEASED_WAIT,
    BUTTON_PRESSED,
    BUTTON_PRESSED_WAIT

};
 
 /* Typedefs --------------------------------------------------------------------*/
 typedef struct fsm_button_t 	fsm_button_t;
 
 /* Function prototypes and explanation -------------------------------------------------*/
 
bool fsm_button_check_activity(fsm_button_t *p_fsm);

/**
 * @brief Elimina un botón FSM
 * 
 * @param p_fsm Puntero a una estructura fsm_button_t
 */

void fsm_button_destroy(fsm_button_t *p_fsm);

/**
 * @brief Se utiliza para activar el botón FSM
 * 
 * @param p_fsm Puntero a una estructura fsm_button_t
 */

void fsm_button_fire(fsm_button_t *p_fsm);

/**
 * @brief Devuelve el debounce time del botón FSM
 * 
 * @param p_fsm Puntero a una estructura fsm_button_t
 * 
 * @return Duración del debounce time en milisegundos
 */

uint32_t fsm_button_get_debounce_time_ms(fsm_button_t *p_fsm);	

/**
 * @brief Devuelve la duración de la última pulsación del botón
 * 
 * @param p_fsm Puntero a una estructura fsm_button_t
 * 
 * @return Duración de la última pulsación del botón en milisegundos
 */

uint32_t fsm_button_get_duration(fsm_button_t *p_fsm);

/**
 * @brief Esta función devuelve el FSM interno del botón
 * 
 * @param p_fsm Puntero a una estructura fsm_button_t
 * 
 * @return Devuelve el puntero al FSM interno
 */

fsm_t* fsm_button_get_inner_fsm(fsm_button_t *p_fsm);

/**
 * @brief Obtiene el estado del botón FSM
 * 
 * @param p_fsm Puntero a una estructura fsm_button_t
 * 
 * @return Devuelve el estado del botón FSM
 */

uint32_t fsm_button_get_state(fsm_button_t *p_fsm);

/**
 * @brief Crea un nuevo botón FSM con el debounce time dado y el button_id
 * 
 * @param button_id Identificador del botón, único
 * @param debounce_time_ms Debounce time en milisegundos
 * 
 * @return Devuelve el puntero al botón FSM
 */

fsm_button_t* fsm_button_new(uint32_t debounce_time_ms, uint32_t button_id);

/**
 * @brief Reinicia la duración de la última pulsación del botón
 * 
 * @param p_fsm Puntero a una estructura fsm_button_t
 */

void fsm_button_reset_duration(fsm_button_t *p_fsm);
 
#endif