/**
 * @file port_ultrasound.h
 * @brief Header for the portable functions to interact with the HW of the ultrasound sensors. The functions must be implemented in the platform-specific code.
 * @author Nicolás Henández Martín
 * @author Gonzalo Leis Varela
 * @date 17/03/2025
 */
#ifndef PORT_ULTRASOUND_H_
#define PORT_ULTRASOUND_H_


/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
/* Standard C includes */

/* Defines and enums ----------------------------------------------------------*/
#define PORT_REAR_PARKING_SENSOR_ID 0   //Se le asigna al sensor el ID 0
#define PORT_PARKING_SENSOR_TRIGGER_UP_US 10    //Tiempo de la señal de disparo
#define PORT_PARKING_SENSOR_TIMEOUT_MS 100  //Tiempo entre una medición y otra
#define SPEED_OF_SOUND_MS 343   //Velocidad del sonido en el aire (m/s)
/* Function prototypes and explanation -------------------------------------------------*/

/**
 * @brief Configura las especificaciones HW del sensor de ultrasonidos 
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 */

void port_ultrasound_init(uint32_t ultrasound_id);

                    //TRIGGER

/**
 * @brief Detiene el temporizador que controla el trigger cuando el tiempo de trigger ha finalizado 
 * Además establece la señal de trigger a 0
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 */

void port_ultrasound_stop_trigger_timer(uint32_t ultrasound_id);

/**
 * @brief Esta función devuelve el estado de preparación de la señal de disparo
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 * 
 * @return TRUE, Sensor preparado para iniciar nueva medición
 * @return FALSE, Sensor no preparado para iniciar nueva medición
 */

bool port_ultrasound_get_trigger_ready(uint32_t ultrasound_id);

/**
 * @brief Esta función establece el estado de preparación de la señal trigger
 * Si trigger_ready, es true, el sensor estará preparado para una nueva medición 
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 * @param trigger_ready Estado de la señal trigger
 */

void port_ultrasound_set_trigger_ready(uint32_t ultrasound_id, bool trigger_ready);

/**
 * @brief Devuelve el estado de la señal de trigger. 
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 * 
 * @return TRUE, Tiempo de trigger del sensor ha terminado
 * @return FALSE, Tiempo de trigger del sensor no ha terminado
 */

bool port_ultrasound_get_trigger_end(uint32_t ultrasound_id);

/**
 * @brief Establece el estado de la señal trigger 
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 * @param trigger_end Estado de la señal trigger. Si es true, el tiempo de trigger habrá acabado
 */

void port_ultrasound_set_trigger_end(uint32_t ultrasound_id, bool trigger_end);
	
                    //ECHO

/**
 * @brief Obtiene el instante de tiempo cuando se recibe (init) el echo
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 * 
 * @return Devuelve el instante de tiempo
 */

uint32_t port_ultrasound_get_echo_init_tick(uint32_t ultrasound_id);

/**
 * @brief Establece el momento en el que llega el echo 
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 * @param echo_init_tick Instante de tiempo a establecer
 */

void port_ultrasound_set_echo_init_tick(uint32_t ultrasound_id, uint32_t echo_init_tick);

/**
 * @brief Obtiene el instante de tiempo cuando termina (end) el echo 
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 * 
 * @return Devuelve el instante de tiempo
 */

uint32_t port_ultrasound_get_echo_end_tick(uint32_t ultrasound_id);

/**
 * @brief Establece el momento en el que acaba el echo 
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 * @param echo_end_tick Instante de tiempo a establecer
 */

void port_ultrasound_set_echo_end_tick(uint32_t ultrasound_id, uint32_t echo_end_tick);

/**
 * @brief Devuelve el estado de la señal echo
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 * 
 * @return true: sí se ha recibido el echo 
 * @return false: no se ha recibido el echo
 */

bool port_ultrasound_get_echo_received(uint32_t ultrasound_id);	

/**
 * @brief Establece el estado de la señal echo
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 * @param echo_received Estado de la señal de echo a establecer
 */

void port_ultrasound_set_echo_received(uint32_t ultrasound_id, bool echo_received);	

/**
 * @brief Obtiene el número de desbordamientos del temporizador que controla la señal echo
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 * 
 * @return Número de desbordamientos 
 */

uint32_t port_ultrasound_get_echo_overflows(uint32_t ultrasound_id);	

/**
 * @brief Establece el número de desbordamientos del temporizador que controla la señal echo
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 * @param echo_overflows Número de desbordamientos
 */

void port_ultrasound_set_echo_overflows(uint32_t ultrasound_id, uint32_t echo_overflows);

/**
 * @brief Para el temporizador que controla el echo cuando el echo ha sido recibido
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 */

void port_ultrasound_stop_echo_timer(uint32_t ultrasound_id);

/**
 * @brief Resetea los ticks del echo cuando la distancia ya ha sido calculada
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 */

void port_ultrasound_reset_echo_ticks(uint32_t ultrasound_id);

                    //TIMEOUT

/**
 * @brief Prepara el timer del trigger y del echo y activa el timer que controla la nueva medida
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 */

void port_ultrasound_start_measurement(uint32_t ultrasound_id);

/**
 * @brief Inicia el timer que controla la nueva medición
 */

void port_ultrasound_start_new_measurement_timer(void);

/**
 * @brief Detiene el timer que controla la nueva medición
 */

void port_ultrasound_stop_new_measurement_timer(void);

/**
 * @brief Para todos los timers y reinicia el echo_ticks
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 */

void port_ultrasound_stop_ultrasound(uint32_t ultrasound_id);	

#endif /* PORT_ULTRASOUND_H_ */
