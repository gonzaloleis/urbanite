/**
 * @file fsm_ultrasound.c
 * @brief Ultrasound sensor FSM main file.
 * @author Nicolás Henández Martín
 * @author Gonzalo Leis Varela
 * @date 10/04/2025
 */

#include <stdlib.h>
#include <stdio.h>

#include "port_system.h"
#include "fsm.h"
#include "fsm_urbanite.h"

struct fsm_urbanite_t{

    fsm_t f;
    fsm_button_t * 	p_fsm_button;
    uint32_t on_off_press_time_ms;
    uint32_t pause_display_time_ms;
    bool is_paused;
    fsm_ultrasound_t * p_fsm_ultrasound_rear;
    fsm_display_t * p_fsm_display_rear;

};

/* State machine input functions */

/**
 * @brief Comprueba si el botón ha sido presionado lo suficiente como para que se inicie el sistema
 *
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_urbanite_t
 * 
 * @return True, Sí ha sido pulsado más de un segundo
 * @return True, No ha sido pulsado más de un segundo
 */

static bool check_on(fsm_t * p_this){
    
    fsm_urbanite_t *p_urbanite = (fsm_urbanite_t*)p_this;
    
    uint32_t time = fsm_button_get_duration(p_urbanite->p_fsm_button);
    
    if(time > 0 && time > p_urbanite->on_off_press_time_ms){
        return true;                         
    }
    return false;

}

/**
 * @brief Comprueba si el botón ha sido presionado lo suficiente como para que se apague el sistema
 *
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_urbanite_t
 * 
 * @return True, Sí ha sido pulsado más de un segundo
 * @return True, No ha sido pulsado más de un segundo
 */

static bool check_off(fsm_t * p_this){
    return check_on(p_this);
}

/**
 * @brief Comprueba si hay una nueva medida
 *
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_urbanite_t
 * 
 * @return True, Sí hay una nueva medida
 * @return True, No hay una nueva medida
 */

static bool check_new_measure(fsm_t * p_this){
    
    fsm_urbanite_t *p_urbanite = (fsm_urbanite_t*)p_this;
    
    return fsm_ultrasound_get_new_measurement_ready(p_urbanite->p_fsm_ultrasound_rear);

}

/**
 * @brief Comprueba si se ha requerido parar el display
 *
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_urbanite_t
 * 
 * @return True, Sí se requiere pausa
 * @return True, No se requiere pausa
 */

static bool check_pause_display(fsm_t * p_this){
    
    fsm_urbanite_t *p_urbanite = (fsm_urbanite_t*)p_this;
    
    uint32_t time = fsm_button_get_duration(p_urbanite->p_fsm_button);
    
    if(time > 0 && time > p_urbanite->pause_display_time_ms && time < p_urbanite->on_off_press_time_ms){
        return true;                                     
    }
    return false;

}

/**
 * @brief Comprueba si cualquiera de los elementos del sistema está activo
 *
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_urbanite_t
 * 
 * @return True, Sí hay algo activo
 * @return True, No hay nada acitvo
 */

static bool check_activity(fsm_t * p_this){
    
    fsm_urbanite_t *p_urbanite = (fsm_urbanite_t*)p_this;
    
    if(fsm_button_check_activity(p_urbanite->p_fsm_button) || fsm_ultrasound_check_activity(p_urbanite->p_fsm_ultrasound_rear) || fsm_display_check_activity(p_urbanite->p_fsm_display_rear)){
        return true;
    }
    return false;

}

/**
 * @brief Comprueba si todos los elementos del sistema está activo
 *
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_urbanite_t
 * 
 * @return True, Sí esán todos activos
 * @return True, No están todos acitvos
 */

static bool check_no_activity(fsm_t * p_this){
    return !check_activity(p_this);
}

/**
 * @brief Comprueba si hay una nueva medida estando en el modo bajo consumo
 *
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_urbanite_t
 * 
 * @return True, Sí hay una medida
 * @return True, No hay una medida
 */

static bool check_activity_in_measure(fsm_t * p_this){
    return check_new_measure(p_this);
}

/* State machine output or action functions */

/**
 * @brief Inicia el sistema
 *
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_urbanite_t
 */

static void do_start_up_measure(fsm_t * p_this){
    
    fsm_urbanite_t *p_urbanite = (fsm_urbanite_t*)p_this;
    
    fsm_button_reset_duration(p_urbanite->p_fsm_button);

    fsm_ultrasound_start(p_urbanite->p_fsm_ultrasound_rear);

    fsm_display_set_status(p_urbanite->p_fsm_display_rear, true);

    printf("[URBANITE][%ld] Urbanite system ON\n", port_system_get_millis());   
}

/**
 * @brief Apaga el sistema
 *
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_urbanite_t
 */

static void do_stop_urbanite(fsm_t * p_this){
    
    fsm_urbanite_t *p_urbanite = (fsm_urbanite_t*)p_this;
    
    fsm_button_reset_duration(p_urbanite->p_fsm_button);

    fsm_ultrasound_stop(p_urbanite->p_fsm_ultrasound_rear);

    fsm_display_set_status(p_urbanite->p_fsm_display_rear, true);

    if(p_urbanite->is_paused){
        p_urbanite->is_paused = false;
    }

    printf("[URBANITE][%ld] Urbanite system OFF\n", port_system_get_millis());
}

/**
 * @brief Pause o Resume, para el sistema
 *
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_urbanite_t
 */

static void do_pause_display(fsm_t * p_this){

    fsm_urbanite_t *p_urbanite = (fsm_urbanite_t*)p_this;
    
    fsm_button_reset_duration(p_urbanite->p_fsm_button);
    
    if(p_urbanite->is_paused){
        p_urbanite->is_paused = false;
        printf("[URBANITE][%ld] Urbanite system display RESUME\n", port_system_get_millis());
    } else{
        p_urbanite->is_paused = true;
        printf("[URBANITE][%ld] Urbanite system display PAUSE\n", port_system_get_millis());
    }

    fsm_display_set_status(p_urbanite->p_fsm_display_rear, p_urbanite->is_paused);
}

/**
 * @brief Presenta la distancia medida por el sensor
 *
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_urbanite_t
 */

static void do_display_distance(fsm_t * p_this){
    
    fsm_urbanite_t *p_urbanite = (fsm_urbanite_t*)p_this;

    uint32_t distance = fsm_ultrasound_get_distance(p_urbanite->p_fsm_ultrasound_rear);

    if(p_urbanite->is_paused){
        if(distance < (DISTANCE_1/2)){
            fsm_display_set_distance(p_urbanite->p_fsm_display_rear, distance);
            fsm_display_set_status(p_urbanite->p_fsm_display_rear, true);
        }
        else{
            fsm_display_set_status(p_urbanite->p_fsm_display_rear, false);
        }
    } else{
        fsm_display_set_distance(p_urbanite->p_fsm_display_rear, distance);
        fsm_display_set_status(p_urbanite->p_fsm_display_rear, true);
    }

    printf("[URBANITE][%ld] Distance: %ld cm\n", port_system_get_millis(), distance);  
}

/**
 * @brief Comienza el modo bajo consumo cuando el sistema esta OFF
 *
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_urbanite_t
 */

static void do_sleep_off(fsm_t * p_this){
    port_system_sleep();
}

/**
 * @brief Comienza el modo bajo consumo cuando el sistema se despierta por un punto de parada o similar
 *
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_urbanite_t
 */

static void do_sleep_while_off(fsm_t * p_this){
    port_system_sleep();
}

/**
 * @brief Comienza el modo bajo consumo cuando el sistema se despierta por un punto de parada o similar
 *
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_urbanite_t
 */

static void do_sleep_while_on(fsm_t * p_this){
    port_system_sleep();
}

/**
 * @brief Comienza el modo bajo consumo cuando el sistema está midiendo y esperando una nueva medida
 *
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_urbanite_t
 */

static void do_sleep_while_measure(fsm_t * p_this){
    port_system_sleep();
}  

// Tabla de transiciones del sistema Urbanite

static fsm_trans_t fsm_trans_urbanite[] = {
    
    {OFF, check_on, MEASURE, do_start_up_measure},
    {OFF, check_no_activity, SLEEP_WHILE_OFF, do_sleep_off},
    {MEASURE, check_new_measure, MEASURE, do_display_distance},
    {MEASURE, check_off, OFF, do_stop_urbanite},
    {MEASURE, check_pause_display, MEASURE, do_pause_display},
    {MEASURE, check_no_activity, SLEEP_WHILE_ON, do_sleep_while_measure},
    {SLEEP_WHILE_ON, check_activity_in_measure, MEASURE, NULL},
    {SLEEP_WHILE_ON, check_no_activity, SLEEP_WHILE_ON, do_sleep_while_on},
    {SLEEP_WHILE_OFF, check_no_activity, SLEEP_WHILE_OFF, do_sleep_while_off},
    {SLEEP_WHILE_OFF, check_activity, OFF, NULL},
    {-1, NULL, -1, NULL}

};

/**
 * @brief Crea una nueva Urbanite FSM
 *
 * @param p_fsm_urbanite Puntero a una FSM Urbanite
 * @param p_fsm_button Puntero a una FSM botón que activa el sistema y desactiva el display
 * @param on_off_press_time_ms Pulsación del botón en milisegundos para apagar o encender el sistema
 * @param pause_display_time_ms Tiempo en milisegundos para parar el display después de una corta pulsación
 * @param p_fsm_ultrasound_rear Puntero a una FSM del sensor que mide la distancia 
 * @param p_fsm_display_rear Puntero a una FSM del display que representa la distancia
 */

static void fsm_urbanite_init(fsm_urbanite_t * p_fsm_urbanite, fsm_button_t * p_fsm_button, uint32_t on_off_press_time_ms, uint32_t pause_display_time_ms, fsm_ultrasound_t * p_fsm_ultrasound_rear, fsm_display_t * p_fsm_display_rear){ 

    // 1- Arrancamos
    fsm_init(&p_fsm_urbanite->f, fsm_trans_urbanite);

    // 2- Inicializamos los valores
    p_fsm_urbanite->is_paused = false;
    p_fsm_urbanite->p_fsm_button = p_fsm_button;
    p_fsm_urbanite->on_off_press_time_ms = on_off_press_time_ms;
    p_fsm_urbanite->p_fsm_ultrasound_rear = p_fsm_ultrasound_rear;
    p_fsm_urbanite->pause_display_time_ms = pause_display_time_ms;
    p_fsm_urbanite->p_fsm_display_rear = p_fsm_display_rear;
}	

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

fsm_urbanite_t* fsm_urbanite_new(fsm_button_t * p_fsm_button, uint32_t on_off_press_time_ms, uint32_t pause_display_time_ms, fsm_ultrasound_t * p_fsm_ultrasound_rear, fsm_display_t * p_fsm_display_rear){
    fsm_urbanite_t * p_fsm_urbanite = malloc(sizeof(fsm_urbanite_t));
    fsm_urbanite_init(p_fsm_urbanite, p_fsm_button, on_off_press_time_ms, pause_display_time_ms, p_fsm_ultrasound_rear, p_fsm_display_rear);
    return p_fsm_urbanite;
}	

/**
 * @brief Se usa para comprobar las transciones 
 *
 * @param p_fsm Puntero a una estructura fsm_urbanite_t
 */

void fsm_urbanite_fire(fsm_urbanite_t * p_fsm){
    fsm_fire(&p_fsm->f);
}

/**
 * @brief Libera la memoria de la FSM
 *
 * @param p_fsm Puntero a una estructura fsm_urbanite_t
 */

void fsm_urbanite_destroy(fsm_urbanite_t * p_fsm){
    free(p_fsm);
}
