/**
 * @file fsm_ultrasound.c
 * @brief Ultrasound sensor FSM main file.
 * @author Nicolás Henández Martín
 * @author Gonzalo Leis Varela
 * @date 17/03/2025
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdlib.h>
#include <string.h>
/* HW dependent includes */
#include "port_ultrasound.h"
#include "port_system.h"
/* Project includes */
#include "fsm.h"
#include "fsm_ultrasound.h"
/* Typedefs --------------------------------------------------------------------*/
/* Private functions -----------------------------------------------------------*/
// Comparison function for qsort
int _compare(const void *a, const void *b)
{
    return (*(uint32_t *)a - *(uint32_t *)b);
}

/* State machine input or transition functions */

/**
 * @brief Comprueba si el sensor está activo y preparado para iniciar una nueva medida
 *
 * @param p_this Puntero a una estructura fsm_t que contiene una fsm_ultrasound_t
 * 
 * @return true: si está activo el flag y por lo tanto preparado para una nueva medida 
 * @return false: no está activo y por lo tanto no está preparado
 */

static bool check_on(fsm_t * p_this){
    
    //Convierte el puntero fsm_t * en un puntero más específico, fsm_ultrasound_t* 
    fsm_ultrasound_t * fsm_ultrasound = (fsm_ultrasound_t *)p_this;
    
    //Devuelve el booleano que confirma si está preparado o no para que se tome una nueva medida
    if(fsm_ultrasound->status && port_ultrasound_get_trigger_ready(fsm_ultrasound->ultrasound_id)){
        return true;
    }
    return false;
}	

/**
 * @brief Comprueba si el sensor se ha deshabilitado
 *
 * @param p_this Puntero a una estructura fsm_t que contiene una fsm_ultrasound_t
 * 
 * @return true: si está inhabilitado
 * @return false: no está inhabilitado
 */

static bool check_off(fsm_t * p_this){
    fsm_ultrasound_t * p_ultrasound = (fsm_ultrasound_t *)p_this;
    return !(p_ultrasound->status);
}

/**
 * @brief Comprueba si el sensor ha terminado la señal de trigger
 *
 * @param p_this Puntero a una estructura fsm_t que contiene una fsm_ultrasound_t
 * 
 * @return true: si ha terminado el trigger
 * @return false: no ha terminado el trigger
 */

static bool check_trigger_end(fsm_t * p_this){
    fsm_ultrasound_t *p_ultrasound = (fsm_ultrasound_t *)p_this;
    //Devuelve el booleano que confirma si ha terminado o no el trigger
    return port_ultrasound_get_trigger_end(p_ultrasound->ultrasound_id);
}

/**
 * @brief Comprueba si el sensor ha recibido el flanco de subida de la señal echo
 *
 * @param p_this Puntero a una estructura fsm_t que contiene una fsm_ultrasound_t
 * 
 * @return true: si ha recibido el flanco de subida de la señal echo
 * @return false: no ha recibido el flanco de subida de la señal echo
 */

static bool check_echo_init(fsm_t * p_this){
    fsm_ultrasound_t *p_ultrasound = (fsm_ultrasound_t *)p_this;
    //El sensor habrá recibido el echo si echo_init_tick > 0
    return (port_ultrasound_get_echo_init_tick(p_ultrasound->ultrasound_id) > 0);
}

/**
 * @brief Comprueba si el sensor ha recibido el flanco de bajada de la señal echo, es decir, ha recibido el echo completo
 *
 * @param p_this Puntero a una estructura fsm_t que contiene una fsm_ultrasound_t
 * 
 * @return true: si ha el echo
 * @return false: no ha recibido el echo
 */

static bool check_echo_received(fsm_t * p_this){
    fsm_ultrasound_t *p_ultrasound = (fsm_ultrasound_t *)p_this;
    
    //El sensor lo habrá recibido si echo_received es true
    return port_ultrasound_get_echo_received(p_ultrasound->ultrasound_id);
}

/**
 * @brief Comprueba si el se puede realizar una nueva medida
 *
 * @param p_this Puntero a una estructura fsm_t que contiene una fsm_ultrasound_t
 * 
 * @return true: si se puede realizar
 * @return false: no se puede realizar
 */

static bool check_new_measurement(fsm_t * p_this){                              
    fsm_ultrasound_t *fsm_ultrasound = (fsm_ultrasound_t *)p_this;                
    return port_ultrasound_get_trigger_ready(fsm_ultrasound->ultrasound_id);
}

/**
 * @brief Comprueba si el sensor está midiendo la distancia
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 * 
 * @return true: SÍ está midiendo
 * @return false: NO está midiendo
 */

bool fsm_ultrasound_check_activity(fsm_ultrasound_t *p_fsm){
    return false;
}

/* State machine output or action functions */

/**
 * @brief Calcula la distancia en cm y la almacena en el array una vez se ha recibido el echo completo
 *
 * @param p_this Puntero a una estructura fsm_t que contiene una fsm_ultrasound_t
 */

static void do_set_distance(fsm_t * p_this){
    fsm_ultrasound_t *p_ultrasound = (fsm_ultrasound_t *)p_this;

    //1- Recuperar ticks de inicio y fin del eco, y el número de desbordamientos 
    uint32_t echo_init = port_ultrasound_get_echo_init_tick(p_ultrasound->ultrasound_id);
    uint32_t echo_end  = port_ultrasound_get_echo_end_tick(p_ultrasound->ultrasound_id);
    uint32_t overflows = port_ultrasound_get_echo_overflows(p_ultrasound->ultrasound_id);

    //2- Calcular el delta de tiempo, teniendo en cuenta los overflows
    uint32_t delta_tick = (echo_end - echo_init) + (overflows * 65535);

    //3- Calcular la distancia en cm
    uint32_t distance = (uint32_t)(delta_tick * SPEED_OF_SOUND_MS)/(2*10000);

    //4- Almacenar la distancia en el array en la posición indicada por distance_idx 
    p_ultrasound->distance_arr[p_ultrasound->distance_idx] = distance;

    //5- Comprobar si se ha completado el array
    if (p_ultrasound->distance_idx == FSM_ULTRASOUND_NUM_MEASUREMENTS - 1) {
        //Ordenamos el array
        qsort(p_ultrasound->distance_arr, FSM_ULTRASOUND_NUM_MEASUREMENTS, sizeof(uint32_t), _compare);

        //6- Calcular la mediana
        uint32_t median;
        if (FSM_ULTRASOUND_NUM_MEASUREMENTS % 2 == 1) {
            median = p_ultrasound->distance_arr[FSM_ULTRASOUND_NUM_MEASUREMENTS / 2];
        } else {
            uint32_t mid1 = p_ultrasound->distance_arr[(FSM_ULTRASOUND_NUM_MEASUREMENTS / 2) - 1];
            uint32_t mid2 = p_ultrasound->distance_arr[FSM_ULTRASOUND_NUM_MEASUREMENTS / 2];
            median = (mid1 + mid2) / 2;
        }
        p_ultrasound->distance_cm = median;
        p_ultrasound->new_measurement = true;
    }

    //8- Incrementar el índice, usando el operador módulo
    p_ultrasound->distance_idx = (p_ultrasound->distance_idx + 1) % FSM_ULTRASOUND_NUM_MEASUREMENTS;

    //9- Detener el timer de eco
    port_ultrasound_stop_echo_timer(p_ultrasound->ultrasound_id);

    //10- Resetear los ticks de eco
    port_ultrasound_reset_echo_ticks(p_ultrasound->ultrasound_id);
}

/**
 * @brief El transceptor comienza una medida por primera vez desde que arranca la FSM
 *
 * @param p_this Puntero a una estructura fsm_t que contiene una fsm_ultrasound_t
 */

static void do_start_measurement(fsm_t *p_this){
    fsm_ultrasound_t *p_ultrasound = (fsm_ultrasound_t *)p_this; 
    port_ultrasound_start_measurement(p_ultrasound->ultrasound_id);
}

/**
 * @brief Se llama a esta función cuando se ha acabado una medida y se quiere empezar una nueva
 *
 * @param p_this Puntero a una estructura fsm_t que contiene una fsm_ultrasound_t
 */

static void do_start_new_measurement(fsm_t * p_this){
    do_start_measurement(p_this);
}

/**
 * @brief Para el sensor y reinicia los echo ticks
 *
 * @param p_this Puntero a una estructura fsm_t que contiene una fsm_ultrasound_t
 */

static void do_stop_measurement(fsm_t * p_this){
    fsm_ultrasound_t *p_ultrasound = (fsm_ultrasound_t *)p_this; 
    port_ultrasound_stop_ultrasound(p_ultrasound->ultrasound_id);
}

/**
 * @brief Para la señal de trigger y su temporizador cuando la señal de trigger acaba
 *
 * @param p_this Puntero a una estructura fsm_t que contiene una fsm_ultrasound_t
 */

static void do_stop_trigger(fsm_t * p_this){
    fsm_ultrasound_t *p_ultrasound = (fsm_ultrasound_t *)p_this; 
    port_ultrasound_stop_trigger_timer(p_ultrasound->ultrasound_id);
    port_ultrasound_set_trigger_end(p_ultrasound->ultrasound_id, false);
}

/*
    Array que representa la matriz de transiciones de la máquina de estados (FSM) del sensor
    Es muy importante el orden, cambiarlo podría hacer que no funcione
    ESTADO ACTUAL --> CONDICIÓN PARA QUE SE DE LA TRANSICIÓN --> ESTADO AL QUE SE CAMBIA --> ACCIÓN A EJECUTAR
*/

static fsm_trans_t fsm_trans_ultrasound[] = {
    {WAIT_START, check_on, TRIGGER_START, do_start_measurement},
    {TRIGGER_START, check_trigger_end, WAIT_ECHO_START, do_stop_trigger},
    {WAIT_ECHO_START, check_echo_init, WAIT_ECHO_END, NULL},
    {WAIT_ECHO_END, check_echo_received, SET_DISTANCE, do_set_distance},
    {SET_DISTANCE, check_new_measurement, TRIGGER_START, do_start_new_measurement},
    {SET_DISTANCE, check_off, WAIT_START, do_stop_measurement},
    {-1, NULL, -1, NULL},
};

/* Other auxiliary functions */

/**
 * @brief Inicializa los valores por defecto de la FSM e inicializa el HW con el ID dado
 *
 * @param p_fsm_ultrasound
 * @param ultrasound_id
 */

 static void fsm_ultrasound_init(fsm_ultrasound_t * p_fsm_ultrasound, uint32_t ultrasound_id){  //CAMBIADA
    
    // 1- Llamamos a fsm_init() para inicializar la FSM 
    fsm_init(&p_fsm_ultrasound->f, fsm_trans_ultrasound);

    // 2- Inicializamos a 0 los valores distance_cm, distance_idx y distance_arr
    p_fsm_ultrasound->distance_cm = 0;
    p_fsm_ultrasound->distance_idx = 0;
    memset(p_fsm_ultrasound->distance_arr, 0, sizeof(p_fsm_ultrasound->distance_arr));

    // 3- Inicializamos a false los flags status y new_measurement
    p_fsm_ultrasound->status = false;
    p_fsm_ultrasound->new_measurement = false;
    p_fsm_ultrasound->ultrasound_id = ultrasound_id;

    // 4- Llamamos a la función port_ultrasound_init() para inicializar el HW del sensor de ultrasonidos
    port_ultrasound_init(ultrasound_id);

}

/* Public functions -----------------------------------------------------------*/
fsm_ultrasound_t *fsm_ultrasound_new(uint32_t ultrasound_id)
{
    fsm_ultrasound_t *p_fsm_ultrasound = malloc(sizeof(fsm_ultrasound_t)); /* Do malloc to reserve memory of all other FSM elements, although it is interpreted as fsm_t (the first element of the structure) */
    fsm_ultrasound_init(p_fsm_ultrasound, ultrasound_id);                  /* Initialize the FSM */
    return p_fsm_ultrasound;
}

/**
 * @brief Activa la FSM del sensor
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 */

void fsm_ultrasound_fire(fsm_ultrasound_t * p_fsm){
    fsm_fire(&p_fsm->f);
}

/**
 * @brief Elimina la FSM de un sensor
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 */

void fsm_ultrasound_destroy(fsm_ultrasound_t * p_fsm){
    free(p_fsm);
}

/**
 * @brief Obtiene el estado actual de la FSM del sensor
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 * 
 * @return Estado actual del sensor
 */

 int32_t fsm_ultrasound_get_state(fsm_ultrasound_t * p_fsm){
    return p_fsm->f.current_state;
}

/**
 * @brief Establece el state del display FSM
 *
 * @param p_fsm Puntero a una estructura fsm_display_t
 */


void fsm_ultrasound_set_state(fsm_ultrasound_t * p_fsm, int8_t state){
    p_fsm->f.current_state = state;
}

/**
 * @brief Obtiene la FSM interna del sensor
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 * 
 * @return Puntero a la FSM interna 
 */

fsm_t* fsm_ultrasound_get_inner_fsm(fsm_ultrasound_t * p_fsm){
    return (fsm_t *)p_fsm;
}

/**
 * @brief Devuelve la distancia del objeto detectado por el sensor
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 * 
 * @return Distancia en cm 
 */

uint32_t fsm_ultrasound_get_distance(fsm_ultrasound_t * p_fsm){
    uint32_t distance = p_fsm->distance_cm;
    p_fsm->new_measurement = false;
    return distance;
}

/**
 * @brief Para el sensor reseteando los ticks (port) y desactivando el sensor
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 */

void fsm_ultrasound_stop(fsm_ultrasound_t * p_fsm){
    p_fsm->status = false;
    port_ultrasound_stop_ultrasound(p_fsm->ultrasound_id);
}

/**
 * @brief Inicializa el sensor reseteando los ticks (port) y activando el sensor
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 */

void fsm_ultrasound_start(fsm_ultrasound_t * p_fsm){
    p_fsm->status = true;
    p_fsm->distance_cm = 0;
    p_fsm->distance_idx = 0;
    port_ultrasound_reset_echo_ticks(p_fsm->ultrasound_id);  
    port_ultrasound_set_trigger_ready(p_fsm->ultrasound_id, true);
    port_ultrasound_start_new_measurement_timer();
}

/**
 * @brief Comprueba si el sensor está activo  inactivo
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 * 
 * @return true: si está activo
 * @return false: no está activo
 */

bool fsm_ultrasound_get_status(fsm_ultrasound_t * p_fsm){
    return p_fsm->status;
}

/**
 * @brief Establece si el sensor está activo o inactivo
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 * @param status Booleano que indica si el sensor está activo (true) o inactivo (false)
 */

void fsm_ultrasound_set_status(fsm_ultrasound_t * p_fsm, bool status){
    p_fsm->status = status;
}

/**
 * @brief Comprueba si el trigger esta preparado 
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 * 
 * @return true: si está preparado
 * @return false: no está preparado
 */

bool fsm_ultrasound_get_ready(fsm_ultrasound_t * p_fsm){
    return port_ultrasound_get_trigger_ready(p_fsm->ultrasound_id);
}

/**
 * @brief Devuelve el booleano que indica si hay una nueva medida preparada
 *
 * @param p_fsm Puntero a una estructura fsm_ultrasound_t
 * 
 * @return true: si la hay
 * @return false: no la hay
 */

bool fsm_ultrasound_get_new_measurement_ready(fsm_ultrasound_t * p_fsm){
    return p_fsm->new_measurement;
}

