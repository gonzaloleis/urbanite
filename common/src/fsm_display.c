/**
 * @file fsm_display.c
 * @brief Display system FSM main file.
 * @author Nicolás Hernández Martín 
 * @author Gonzalo Leis Varela
 * @date 11/05/2025
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdlib.h>
#include <stdio.h>
#include "fsm.h"
/* HW dependent includes */
#include "fsm_display.h"
#include "port_display.h"
#include "port_system.h"
/* Project includes */

/* Typedefs --------------------------------------------------------------------*/
struct fsm_display_t{
    
    fsm_t f;                //Máquina de estados
    int32_t distance_cm;    //Distancia al objeto
    bool new_color;         //Indica si se tiene que establecer un nuevo color
    bool status;            //Indica si el display está activo
    bool idle;              //Indica si el display activo es idle o no
    uint32_t display_id;    //ID del display

};
/* Private functions -----------------------------------------------------------*/

/**
 * @brief Establezca los niveles de color de los LED RGB según la distancia
 *
 * @param p_color Puntero a una estructura rgb_color_t que almacena los valores del LED RGB
 * @param distance_cm Distancia medida por el sensor de ultrasonidos
 */

void _compute_display_levels(rgb_color_t * p_color, int32_t distance_cm){

    if(distance_cm >= DISTANCE_0 && distance_cm <= DISTANCE_1){
        *p_color = COLOR_RED;
    }

    else if(distance_cm > DISTANCE_1 && distance_cm <= DISTANCE_2){
        *p_color = COLOR_ORANGE;
    }

    else if(distance_cm > DISTANCE_2 && distance_cm <= DISTANCE_3){
        *p_color = COLOR_YELLOW;
    }

    else if(distance_cm > DISTANCE_3 && distance_cm <= DISTANCE_4){
        *p_color = COLOR_LIGHT_GREEN;
    }

    else if(distance_cm > DISTANCE_4 && distance_cm <= DISTANCE_5){
        *p_color = COLOR_GREEN;
    }

    else if(distance_cm > DISTANCE_5 && distance_cm <= DISTANCE_6){
        *p_color = COLOR_CYAN;
    }

    else if(distance_cm > DISTANCE_6 && distance_cm <= DISTANCE_7){
        *p_color = COLOR_LIGHT_BLUE;
    }

    else if(distance_cm > DISTANCE_7 && distance_cm <= DISTANCE_8){
        *p_color = COLOR_BLUE;
    }

    else if(distance_cm > DISTANCE_8 && distance_cm <= DISTANCE_9){
        *p_color = COLOR_DARK_BLUE;
    }

    else if(distance_cm > DISTANCE_9 && distance_cm <= DISTANCE_10){
        *p_color = COLOR_PURPLE;
    }

    else{
        *p_color = COLOR_OFF;
    }
    
} 

/* State machine input or transition functions */

/**
 * @brief Comprueba si el display está activo o no independientemente del idle
 *
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_display_t
 */

static bool check_active(fsm_t * p_this){
    fsm_display_t *display = (fsm_display_t *)p_this;
    return display->status;
}	

/**
 * @brief Comprueba si hay que establecer un nuevo color
 *
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_display_t
 */

static bool check_set_new_color(fsm_t * p_this){
    fsm_display_t *display = (fsm_display_t *)p_this;
    return display->new_color;
}

/**
 * @brief Comprueba si el display está inactivo
 *
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_display_t
 */

static bool check_off(fsm_t * p_this){
    fsm_display_t *display = (fsm_display_t *)p_this;
    return !(display->status);
}

bool fsm_display_check_activity (fsm_display_t *p_fsm){
    if(p_fsm->status == true && p_fsm->idle == false){
        return true;
    } else{
        return false;
    }

}

/* State machine output or action functions */

/**
 * @brief Activa el sensor por primera vez
 *
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_display_t
 */

static void do_set_on(fsm_t * p_this){
    fsm_display_t *display = (fsm_display_t *)p_this;
    port_display_set_rgb(display->display_id, COLOR_OFF); 
}

/**
 * @brief Establece el color del LED RGB en función de la distancia medida
 *
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_display_t
 */

static void do_set_color(fsm_t * p_this){

    fsm_display_t *display = (fsm_display_t *)p_this;
    rgb_color_t color;

    // 1- Calcular los niveles de los LED RGB según la distancia y establecer el nivel de visualización
    _compute_display_levels(&color, display->distance_cm);

    // 2- Llamamos a la función con el ID del LED RGB y el color
    port_display_set_rgb(display->display_id, color);

    // 3- Indicamos que el color ya ha sido establecido 
    display->new_color = false;

    // 4- Indiamos que el sistema está en modo bajo consumo (idle activado)
    display->idle = true;

}

/**
 * @brief Desactiva el display
 *
 * @param p_this Puntero a una estructura fsm_t que contiene un fsm_display_t
 */

static void do_set_off(fsm_t * p_this){
    fsm_display_t *display = (fsm_display_t *)p_this;
    port_display_set_rgb(display->display_id, COLOR_OFF); 
    display->idle = false;
}

/* Other auxiliary functions */

static fsm_trans_t fsm_trans_display[] = {
    {WAIT_DISPLAY, check_active, SET_DISPLAY, do_set_on},
    {SET_DISPLAY, check_set_new_color, SET_DISPLAY, do_set_color},
    {SET_DISPLAY, check_off, WAIT_DISPLAY, do_set_off},
    {-1, NULL, -1, NULL}
};

/**
 * @brief Inicializa el display
 *
 * @param p_fsm_display Puntero al display fsm
 * @param display_id ID del display
 */

static void fsm_display_init(fsm_display_t * p_fsm_display, uint32_t display_id){
    
    // 1- Inicializamos la fsm
    fsm_init(&p_fsm_display->f, fsm_trans_display);

    // 2- Inicilizamos distance_idx
    p_fsm_display->display_id = display_id;
    
    // 3- Inicializamos la distancia a un valor fuera del rango
    p_fsm_display->distance_cm = -1;

    // 4- Inicializamos los flags del p_fsm_display
    p_fsm_display->new_color = false;
    p_fsm_display->status = false;
    p_fsm_display->idle = false;

    // 5- Inicializamos el Hardware
    port_display_init(display_id);

}

/**
 * @brief Elimina la estructura fsm
 *
 * @param p_fsm Puntero a una estructura fsm_display_t
 */

void fsm_display_destroy(fsm_display_t * p_fsm){
    free(p_fsm);
}

/**
 * @brief Ejecuta las transiciones
 *
 * @param p_fsm Puntero a una estructura fsm_display_t
 */

void fsm_display_fire(fsm_display_t * p_fsm){
    fsm_fire(&p_fsm->f);
}

/**
 * @brief Obtiene la estructura fsm interna del display
 *
 * @param p_fsm Puntero a una estructura fsm_display_t
 */

fsm_t* fsm_display_get_inner_fsm(fsm_display_t * p_fsm){
    return (fsm_t *)p_fsm;
}

/**
 * @brief Obtiene el estado del display fsm
 *
 * @param p_fsm Puntero a una estructura fsm_display_t
 */

uint32_t fsm_display_get_state(fsm_display_t * p_fsm){
    return p_fsm->f.current_state;
}

/**
 * @brief Obtiene el status del display fsm
 *
 * @param p_fsm Puntero a una estructura fsm_display_t
 */

bool fsm_display_get_status(fsm_display_t * p_fsm){
    return p_fsm->status;
}

/**
 * @brief Establece el status de la fsm
 * 
 * @param p_fsm Puntero a una estructura fsm_display_t
 * @param pause true, si el sistema se ha pausado
 */

void fsm_display_set_status(fsm_display_t * p_fsm, bool pause){
    p_fsm->status = pause;
}

/**
 * @brief Obtiene la distancia
 *
 * @param p_fsm Puntero a una estructura fsm_display_t
 */

int32_t fsm_display_get_distance(fsm_display_t * p_fsm){
    return p_fsm->distance_cm;
}

/**
 * @brief Configura la visualización para que se muestre la distancia
 *
 * @param p_fsm Puntero a una estructura fsm_display_t
 */

void fsm_display_set_distance(fsm_display_t * p_fsm, uint32_t distance_cm){
    p_fsm->distance_cm = distance_cm;
    p_fsm->new_color = true;
}

/**
 * @brief Establece el state del display FSM
 *
 * @param p_fsm Puntero a una estructura fsm_display_t
 */

void fsm_display_set_state(fsm_display_t * p_fsm, int8_t state){
    p_fsm->f.current_state = state;
}

/* Public functions -----------------------------------------------------------*/



fsm_display_t *fsm_display_new(uint32_t display_id)
{
    fsm_display_t *p_fsm_display = malloc(sizeof(fsm_display_t)); /* Do malloc to reserve memory of all other FSM elements, although it is interpreted as fsm_t (the first element of the structure) */
    fsm_display_init(p_fsm_display, display_id); /* Initialize the FSM */
    return p_fsm_display;
}