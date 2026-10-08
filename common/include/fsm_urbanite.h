/**
 * @file fsm_button.h
 * @brief Header for fsm_button.c file.
 * @author Nicolás Henández Martín
 * @author Gonzalo Leis Varela
 * @date 10/04/2025
*/

#ifndef FSM_URBANITE_H_
#define FSM_URBANITE_H_

#include <stdint.h>
#include "fsm_button.h"
#include "fsm_display.h"
#include "fsm_ultrasound.h"

enum FSM_URBANITE {
    OFF = 0,
    MEASURE,
    SLEEP_WHILE_OFF,
    SLEEP_WHILE_ON
};

typedef struct fsm_urbanite_t fsm_urbanite_t;

/**
 * @brief Libera la memoria de la FSM
 *
 * @param p_fsm Puntero a una estructura fsm_urbanite_t
 */

void fsm_urbanite_destroy(fsm_urbanite_t * p_fsm);

/**
 * @brief Se usa para comprobar las transciones 
 *
 * @param p_fsm Puntero a una estructura fsm_urbanite_t
 */

void fsm_urbanite_fire(fsm_urbanite_t * p_fsm);

/**
 * @brief Crea una nueva estructura Urbanite 
 *
 * @param p_fsm_button Puntero a una estructura fsm_button_t
 * @param on_off_press_time_ms Tiempo para considerar el ON/OFF del sistema URBANITE
 * @param pause_display_time_ms Tiempo para parar el display del sistema
 * @param p_fsm_ultrasound_rear Puntero a una estructura fsm_ultrasound_t
 * @param p_fsm_display_rear Puntero a una estructura fsm_display_t
 * 
 * @return fsm_urbanite_t: Puntero a una estructura FSM
 */

fsm_urbanite_t* fsm_urbanite_new(fsm_button_t * p_fsm_button, uint32_t on_off_press_time_ms, uint32_t pause_display_time_ms, fsm_ultrasound_t * p_fsm_ultrasound_rear, fsm_display_t * p_fsm_display_rear);

#endif