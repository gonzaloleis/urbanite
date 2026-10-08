/**
 * @file fsm_display.h
 * @brief Header for fsm_display.c file.
 * @author Nicolás Hernández Martín
 * @author Gonzalo Leis Varela
 * @date 11/05/2025
 */

#ifndef FSM_DISPLAY_SYSTEM_H_
#define FSM_DISPLAY_SYSTEM_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include "fsm.h"
#include <stdint.h>
#include <stdbool.h>
/* Defines and enums ----------------------------------------------------------*/ 

#define DISTANCE_0 0
#define DISTANCE_1 25
#define DISTANCE_2 50
#define DISTANCE_3 75
#define DISTANCE_4 100
#define DISTANCE_5 125
#define DISTANCE_6 150
#define DISTANCE_7 175
#define DISTANCE_8 200
#define DISTANCE_9 225
#define DISTANCE_10 250

/* Enums */
enum FSM_DISPLAY_SYSTEM {

    WAIT_DISPLAY = 0,
    SET_DISPLAY

};
/* Defines and enums ----------------------------------------------------------*/

/* Typedefs --------------------------------------------------------------------*/
typedef struct fsm_display_t fsm_display_t;

/* Function prototypes and explanation -------------------------------------------------*/

/**
 * @brief Comprueba si el sistema está activo
 * 
 * @param p_fsm Puntero a una estructura fsm_display_t
 * 
 * @return true, si el sistema es activo y no es idle
 * @return false, el sistema está inactivo o idle
 */

bool fsm_display_check_activity(fsm_display_t * p_fsm);

/**
 * @brief Elimina un display fsm
 * 
 * @param p_fsm Puntero a una estructura fsm_display_t
 */

void fsm_display_destroy(fsm_display_t * p_fsm);

/**
 * @brief Se usa para comprobar las transiciones entre estados
 * @param p_fsm Puntero a una estructura fsm_display_t
 */

void fsm_display_fire(fsm_display_t * p_fsm);

/**
 * @brief Obtiene la fsm interna de un display
 * 
 * @param p_fsm Puntero a una estructura fsm_display_t
 * 
 * @return Puntero a la estructura interna
 */

fsm_t* fsm_display_get_inner_fsm(fsm_display_t * p_fsm);

/**
 * @brief Obtiene el estado actual del fsm
 * 
 * @param p_fsm Puntero a una estructura fsm_display_t
 * 
 * @return Estado actual del sistema
 */

uint32_t fsm_display_get_state(fsm_display_t * p_fsm);

/**
 * @brief Obtiene el status del fsm
 * 
 * @param p_fsm Puntero a una estructura fsm_display_t
 * 
 * @return true, si el display ha indicado que está activo
 * @return false, el display ha indicado que está pausado
 */

bool fsm_display_get_status(fsm_display_t * p_fsm);

/**
 * @brief Crea un nuevo display fsm
 * 
 * @param p_fsm Puntero a una estructura fsm_display_t
 * 
 * @return Puntero al nuevo display
 */

fsm_display_t* fsm_display_new(uint32_t display_id);

/**
 * @brief Esta función se utiliza para configurar el sistema de visualización para que muestre la distancia en cm
 * 
 * @param p_fsm Puntero a una estructura fsm_display_t
 * @param distance_cm Distancia a establecer en el display
 */

void fsm_display_set_distance(fsm_display_t * p_fsm, uint32_t distance_cm);	

/**
 * @brief Establece el estado de la fsm
 * 
 * @param p_fsm Puntero a una estructura fsm_display_t
 * @param state Nuevo estado de la fsm
 */

void fsm_display_set_state(fsm_display_t * p_fsm, int8_t state);

/**
 * @brief Establece el status de la fsm
 * 
 * @param p_fsm Puntero a una estructura fsm_display_t
 * @param pause true, si el sistema se ha pausado
 */

void fsm_display_set_status(fsm_display_t * p_fsm, bool pause);	

#endif /* FSM_DISPLAY_SYSTEM_H_ */