/**
 * @file fsm_button.c
 * @brief Button FSM main file.
 * @author Nicolás Henández Martín
 * @author Gonzalo Leis Varela
 * @date 17/03/2025
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdbool.h>
#include <stdlib.h>
/* HW dependent includes */
#include "port_button.h"
#include "port_system.h"

/* Project includes */
#include "fsm_button.h"

/* State machine input or transition functions */
struct fsm_button_t{
    fsm_t f;
    uint32_t button_id;
    uint32_t debounce_time_ms;
    uint32_t duration;
    uint32_t next_timeout;
    uint32_t tick_pressed;
};

/* State machine output or action functions */

/**
 * @brief Comprueba si el botón fue presionado
 * 
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_button_t
 * 
 * @return TRUE, Sí ha sido presionado 
 * @return FALSE, No ha sido presionado
 */

static bool check_button_pressed(fsm_t* p_this){
    fsm_button_t *p_fsm = (fsm_button_t *)(p_this);
    return port_button_get_pressed(p_fsm->button_id);
}

/**
 * @brief Comprueba si el botón fue liberado
 * 
 * @param p_this Puntero a una estructura fsm_t que un  fsm_button_t
 * 
 * @return TRUE, Sí ha sido liberado 
 * @return FALSE, No ha sido liberado
 */

static bool check_button_released(fsm_t* p_this){
    fsm_button_t *p_fsm = (fsm_button_t *)(p_this);
    return !port_button_get_pressed(p_fsm->button_id);
}

/**
 * @brief Comprueba si el debounce-time ha pasado
 * 
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_button_t
 * 
 * @return TRUE, Sí ha pasado el tiempo 
 * @return FALSE, No ha pasado el tiempo
 */

static bool check_timeout(fsm_t* p_this){
    fsm_button_t *p_fsm = (fsm_button_t *)(p_this);
    uint32_t x = port_system_get_millis();
    if (x > (p_fsm -> next_timeout))
    {
        return true;
    }
    return false;   
}

/**
 * @brief Comprueba si el botón FSM está activo o no 
 * 
 * @param p_fsm Puntero a una estructura fsm_button_t
 * 
 * @return TRUE, Sí está actico
 * @return FALSE, No está activo
 */

bool fsm_button_check_activity(fsm_button_t * p_fsm){
    if(p_fsm->f.current_state == BUTTON_RELEASED){
        return false;
    } else{
        return true;
    }
}

/**
 * @brief Acumula la duración del pulsado del botón 
 * 
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_button_t
 */

static void do_set_duration(fsm_t* p_this){
    fsm_button_t *p_fsm = (fsm_button_t *)(p_this);
    uint32_t x = port_system_get_millis();
    p_fsm -> duration = x - p_fsm->tick_pressed;
    p_fsm -> next_timeout = x + p_fsm->debounce_time_ms;
}

static void do_store_tick_pressed(fsm_t * p_this);

//  Array que representa la tabla de transiciones del botón FSM

static fsm_trans_t fsm_trans_button[] = {
    {BUTTON_RELEASED, check_button_pressed, BUTTON_PRESSED_WAIT, do_store_tick_pressed},
    {BUTTON_PRESSED_WAIT, check_timeout, BUTTON_PRESSED, NULL},
    {BUTTON_PRESSED, check_button_released, BUTTON_RELEASED_WAIT, do_set_duration},
    {BUTTON_RELEASED_WAIT, check_timeout, BUTTON_RELEASED, NULL},
    {-1, NULL, -1,NULL}
};


/**
 * @brief Almacena el tick del sistema cuando se presionó el botón
 * 
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_button_t
 */

static void do_store_tick_pressed(fsm_t * p_this){
    fsm_button_t *p_fsm = (fsm_button_t *)(p_this);
    uint32_t x = port_system_get_millis();
    p_fsm -> tick_pressed = x;
    p_fsm->next_timeout = p_fsm->tick_pressed + p_fsm->debounce_time_ms;
}	

/* Other auxiliary functions */

/**
 * @brief Inicializa un botón FSM.
 * Esta función inicializa los valores predeterminados de la estructura FSM y llama al puerto para inicializar el hardware asociado según el ID
 * 
 * @param p_fsm_button Puntero al botón FSM
 * @param debounce_time Tiempo antirrebotes en milisegundos
 * @param button_id Número de identificación único del botón
 */

void fsm_button_init(fsm_button_t *p_fsm_button, uint32_t debounce_time, uint32_t button_id){
    fsm_init(&p_fsm_button->f, fsm_trans_button);

    p_fsm_button->debounce_time_ms = debounce_time;
    p_fsm_button->button_id = button_id;

    p_fsm_button->tick_pressed = 0;
    p_fsm_button->duration = 0;

    port_button_init(button_id);
}

/* Public functions -----------------------------------------------------------*/

/**
 * @brief Devuelve la duración de la última pulsación del botón
 * 
 * @param p_fsm Puntero a una estructura fsm_button_t
 * 
 * @return Duración de la última pulsación del botón en milisegundos
 */

uint32_t fsm_button_get_duration(fsm_button_t *p_fsm) {
    return p_fsm->duration;
}

/**
 * @brief Reinicia la duración de la última pulsación del botón
 * 
 * @param p_fsm Puntero a una estructura fsm_button_t
 */

void fsm_button_reset_duration(fsm_button_t *p_fsm) {
    p_fsm->duration = 0;
}

/**
 * @brief Devuelve el debounce time del botón FSM
 * 
 * @param p_fsm Puntero a una estructura fsm_button_t
 * 
 * @return Duración del debounce time en milisegundos
 */

uint32_t fsm_button_get_debounce_time_ms(fsm_button_t *p_fsm) {
    return p_fsm->debounce_time_ms;
}

/**
 * @brief Crea un nuevo botón FSM con el debounce time dado y el button_id
 * 
 * @param button_id Identificador del botón, único
 * @param debounce_time_ms Debounce time en milisegundos
 * 
 * @return Devuelve el puntero al botón FSM
 */

fsm_button_t *fsm_button_new(uint32_t debounce_time, uint32_t button_id){
    fsm_button_t *p_fsm_button = malloc(sizeof(fsm_button_t)); /* Do malloc to reserve memory of all other FSM elements, although it is interpreted as fsm_t (the first element of the structure) */
    fsm_button_init(p_fsm_button, debounce_time, button_id);   /* Initialize the FSM */
    return p_fsm_button;                                       /* Composite pattern: return the fsm_t pointer as a fsm_button_t pointer */
}

/* FSM-interface functions. These functions are used to interact with the FSM */

/**
 * @brief Se utiliza para activar el botón FSM
 * 
 * @param p_fsm Puntero a una estructura fsm_button_t
 */

void fsm_button_fire(fsm_button_t *p_fsm){
    fsm_fire(&p_fsm->f); // Is it also possible to it in this way: fsm_fire((fsm_t *)p_fsm);
}

/**
 * @brief Elimina un botón FSM
 * 
 * @param p_fsm Puntero a una estructura fsm_button_t
 */

void fsm_button_destroy(fsm_button_t *p_fsm){
    free(p_fsm);
}

/**
 * @brief Esta función devuelve el FSM interno del botón
 * 
 * @param p_fsm Puntero a una estructura fsm_button_t
 * 
 * @return Devuelve el puntero al FSM interno
 */

fsm_t *fsm_button_get_inner_fsm(fsm_button_t *p_fsm){
    //return &(p_fsm->f);
    return (fsm_t *)p_fsm;
}

/**
 * @brief Obtiene el estado del botón FSM
 * 
 * @param p_fsm Puntero a una estructura fsm_button_t
 * 
 * @return Devuelve el estado del botón FSM
 */

uint32_t fsm_button_get_state(fsm_button_t *p_fsm)
{
    return p_fsm->f.current_state;
}