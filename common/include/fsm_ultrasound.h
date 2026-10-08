/**
 * @file fsm_ultrasound.h
 * @brief Header for fsm_ultrasound.c file.
 * @author Nicolás Henández Martín
 * @author Gonzalo Leis Varela
 * @date 17/03/2025
 */

#ifndef FSM_ULTRASOUND_H_
#define FSM_ULTRASOUND_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "fsm.h"
/* Standard C includes */

/* Defines and enums ----------------------------------------------------------*/
#define FSM_ULTRASOUND_NUM_MEASUREMENTS 5

enum FSM_ULTRASOUND {
    WAIT_START = 0,     //Estado de inicio  
    TRIGGER_START,      //Estado de cuando se envia el trigger
    WAIT_ECHO_START,    //Estado que espera a que llegue el echo
    WAIT_ECHO_END,      //Estado que espera a que termine la señal de echo  
    SET_DISTANCE        //Estado en el que se calcula la distancia de la señal echo
};

/* Typedefs --------------------------------------------------------------------*/
typedef struct fsm_ultrasound_t{

    fsm_t f;                    //Máquina de estados del ultrasonidos
    uint32_t distance_cm;       //Distancia en "cm" al obstáculo
    bool status;                //Indica si se está aparcando o no 
    bool new_measurement;       //Indica si se ha completado una nueva medida
    uint32_t ultrasound_id;     //Id del sensor
    uint32_t distance_arr[FSM_ULTRASOUND_NUM_MEASUREMENTS];     //Array que almacena la última distancia medida
    uint32_t distance_idx;      //Índice para almacenar la última distancia

} fsm_ultrasound_t;
/* Function prototypes and explanation -------------------------------------------------*/

/**
 * @brief Establece el estado actual dentro de la FSM del sensor
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 * @param state Nuevo estado a establecer de la FSM
 */

void fsm_ultrasound_set_state(fsm_ultrasound_t *p_fsm, int8_t state);

/**
 * @brief Comprueba si el sensor está realizando una medida
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 * 
 * @return false: devuelve false siempre 
 */

bool fsm_ultrasound_check_activity(fsm_ultrasound_t * p_fsm);

/**
 * @brief Elimina la FSM de un sensor
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 */

void fsm_ultrasound_destroy(fsm_ultrasound_t * p_fsm);

/**
 * @brief Activa la FSM del sensor
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 */

void fsm_ultrasound_fire(fsm_ultrasound_t * p_fsm);

/**
 * @brief Devuelve la distancia del objeto detectado por el sensor
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 * 
 * @return Distancia en cm 
 */

uint32_t fsm_ultrasound_get_distance(fsm_ultrasound_t * p_fsm);

/**
 * @brief Obtiene la FSM interna del sensor
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 * 
 * @return Puntero a la FSM interna 
 */

fsm_t* fsm_ultrasound_get_inner_fsm(fsm_ultrasound_t * p_fsm);

/**
 * @brief Devuelve el booleano que indica si hay una nueva medida preparada
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 * 
 * @return true: si la hay
 * @return false: no la hay
 */

bool fsm_ultrasound_get_new_measurement_ready(fsm_ultrasound_t * p_fsm);

/**
 * @brief Comprueba si el trigger esta preparado 
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 * 
 * @return true: si está preparado
 * @return false: no está preparado
 */

bool fsm_ultrasound_get_ready(fsm_ultrasound_t * p_fsm);

/**
 * @brief Obtiene el estado actual de la FSM del sensor
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 * 
 * @return Estado actual del sensor
 */

int32_t fsm_ultrasound_get_state(fsm_ultrasound_t * p_fsm);

/**
 * @brief Comprueba si el sensor está activo  inactivo
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 * 
 * @return true: si está activo
 * @return false: no está activo
 */

bool fsm_ultrasound_get_status(fsm_ultrasound_t * p_fsm);

/**
 * @brief Crea un nuevo sensor FSM
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 * 
 * @return Puntero al nuevo sensor
 */

fsm_ultrasound_t* fsm_ultrasound_new(uint32_t ultrasound_id);

/**
 * @brief Establece el estado actual del sensor
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 * @param state Nuevo estado del sensor
 */

void fsm_ultrasound_set_state(fsm_ultrasound_t * p_fsm, int8_t state);

/**
 * @brief Establece si el sensor está activo o inactivo
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 * @param status Booleano que indica si el sensor está activo (true) o inactivo (false)
 */

void fsm_ultrasound_set_status(fsm_ultrasound_t * p_fsm, bool status);

/**
 * @brief Inicializa el sensor reseteando los ticks (port) y activando el sensor
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 */

void fsm_ultrasound_start(fsm_ultrasound_t * p_fsm);

/**
 * @brief Para el sensor reseteando los ticks (port) y desactivando el sensor
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 */

void fsm_ultrasound_stop(fsm_ultrasound_t * p_fsm);

#endif /* FSM_ULTRASOUND_H_ */
