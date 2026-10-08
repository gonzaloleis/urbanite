/**
 * @file stm32f4_ultrasound.c
 * @brief Portable functions to interact with the ultrasound FSM library. All portable functions must be implemented in this file.
 * @author Nicolás Henández Martín
 * @author Gonzalo Leis Varela
 * @date 17/03/2025
 */

/* Standard C includes */
#include <stdio.h>
#include <math.h>
/* HW dependent includes */
#include "port_ultrasound.h"
#include "port_system.h"
#include "stm32f4_system.h"
#include "stm32f4_ultrasound.h"
/* Microcontroller dependent includes */

/* Typedefs --------------------------------------------------------------------*/
typedef struct
{

    GPIO_TypeDef *p_trigger_port;   //GPIO asocidada al trigger
    GPIO_TypeDef *p_echo_port;     //GPIO asociada al echo
    uint8_t trigger_pin;    
    uint8_t echo_pin;       
    uint8_t echo_alt_fun;   
    bool trigger_ready; //Indica si está preparado para lanzar el Trigger
    bool trigger_end;   //Indica si ha acabado ya la señal trigger
    bool echo_received; //Indica si ha llegado el echo
    uint32_t echo_init_tick;    //Almacena el momento de llegada del echo
    uint32_t echo_end_tick;     //Almacena el momento de fin del echo
    uint32_t echo_overflows;    //Almacena el número de desbordamientos del temporizador durante el eco

} stm32f4_ultrasound_hw_t;

/* Global variables */
static stm32f4_ultrasound_hw_t ultrasounds_arr[] = {
    [PORT_REAR_PARKING_SENSOR_ID] = {.p_trigger_port = STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO, .p_echo_port = STM32F4_REAR_PARKING_SENSOR_ECHO_GPIO, .trigger_pin = STM32F4_REAR_PARKING_SENSOR_TRIGGER_PIN, .echo_pin = STM32F4_REAR_PARKING_SENSOR_ECHO_PIN, .echo_alt_fun = 1},
};
/* Private functions ----------------------------------------------------------*/

/**
 * @brief Obtiene la estructura del transceptor el cual se le pasa su ID
 *
 * @param ultrasound_id ID del transceptor
 * 
 * @return Devuelve la estructura del transceptor
 */

stm32f4_ultrasound_hw_t* _stm32f4_ultrasound_get(uint32_t ultrasound_id){
    if(ultrasound_id < sizeof(ultrasounds_arr)/sizeof(ultrasounds_arr[0])){
        return & ultrasounds_arr[ultrasound_id];
    }
    else{
        return NULL;
    }
};
/* Public functions -----------------------------------------------------------*/

/**
 * @brief Configura el HW del sensor de ultrasonidos pasado como parámetro
 *
 * @param ultrasound_id ID del transceptor usado para obtenerlo a partir del array de sensores
 */

/*
                                        TRIGGER
*/

/**
 * @brief Configura el temporizador que controla la duracion de la señal de disparo, trigger
 */

 static void _timer_trigger_setup(){
    
    // 1- Habilitamos el TIMER3, que controla la señal de trigger. El TIMER está conectado al APB1
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;                 
    
    // 2- Apagamos el contador del temporizador para configurarlo nosotros
    TIM3->CR1 &= TIM_CR1_CEN;
    
    // 3- Configuramos el ARPE para que cuando el TIMER llegue al máximo, se reinicie automáticamente
    TIM3->CR1 |= TIM_CR1_ARPE;
    
    // 4- Ponemos el contador de TIMER a 0 para empezar a contar desde 0
    TIM3->CNT = 0;
    
    // 5- Calculamos los valores del prescalado (PSC) y el registro de autorecarga (ARR) para establecer la duración del trigger

    /* 6A- Garantizamos que las siguientes operaciones se hacen con decimales en vez de con enteros
           Establecemos el valor de ARR, 0xFFFF --> 65535  
           Pasamos el tiempo de duración del trigger de milisegundos a segundos  
    */ 
    double doubleSystemCoreClock = (double)SystemCoreClock;
    double ARR = 65535;     
    double triggerTimeS = (double)(1e-6 * PORT_PARKING_SENSOR_TRIGGER_UP_US);

    // 6B- Establecemos un valor inicial para el prescalado y lo asignamos al timer
    double prescaler = round((doubleSystemCoreClock * triggerTimeS)/(ARR + 1) - 1);       //¡¡Fórmula SDG1!!
    TIM3->PSC = prescaler;

    // 6C- Recalculamos el valor del ARR que tendremos ahora que hemos establecido el PSC
    double ARRnew = round((doubleSystemCoreClock * triggerTimeS)/(prescaler + 1) - 1);
    TIM3->ARR = ARRnew;

    /*6D- Comprobamos si el valor de ARR es mayor a 65535
          Si es así, aumentamos el valor de PSC en 1 y recalculamos el ARR
          Si no es así, sirve el valor de ARR calculado anteriormente  
    */ 
    if(ARRnew > 65535){
        prescaler++;
        double ARRnew = round((doubleSystemCoreClock * triggerTimeS)/(prescaler + 1) - 1);
        TIM3->PSC = prescaler;
        TIM3->ARR = ARRnew;
    }

    // 7- Aunque los valores de PSC y ARR ya están configurados en sus registros, necesitamos actualizar para que se guarden bien
    TIM3->EGR |= TIM_EGR_UG;

    // 8- Borramos el bit UIF del registro SR por si pudiera causar una interrupción indeseada
    TIM3->SR = ~ TIM_SR_UIF;

    // 9- Habilitamos las interrupciones del TIMER3 estableciendo el bit UIE del registro DIER
    TIM3->DIER |= TIM_DIER_UIE;

    // 10- Establecemos la prioridad del TIMER3. Prioridad 4, subprioridad 0
    NVIC_SetPriority(TIM3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 4, 0));

}

/**
 * @brief Detiene el temporizador que controla la señal de trigger
 *
 * @param ultrasound_id ID del transceptor usado para obtenerlo a partir del array de sensores
 */

void port_ultrasound_stop_trigger_timer(uint32_t ultrasound_id){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);                                            
    stm32f4_system_gpio_write(p_ultrasound->p_trigger_port, p_ultrasound->trigger_pin, false);
    if(p_ultrasound->trigger_end){
        TIM3->CR1 &= ~ TIM_CR1_CEN;
        NVIC_DisableIRQ(TIM3_IRQn);
    }
}

/**
 * @brief Getter de trigger_ready
 *
 * @param ultrasound_id ID del transceptor usado para obtenerlo a partir del array de sensores
 */

bool port_ultrasound_get_trigger_ready(uint32_t ultrasound_id){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id); 
    return p_ultrasound->trigger_ready;
}

/**
 * @brief Getter de trigger_end
 *
 * @param ultrasound_id ID del transceptor usado para obtenerlo a partir del array de sensores
 */

 bool port_ultrasound_get_trigger_end(uint32_t ultrasound_id){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id); 
    return p_ultrasound->trigger_end;
}

/**
 * @brief Setter de trigger_ready
 *
 * @param ultrasound_id ID del transceptor usado para obtenerlo a partir del array de sensores
 */

 void port_ultrasound_set_trigger_ready(uint32_t ultrasound_id, bool trigger_ready){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id); 
    p_ultrasound->trigger_ready = trigger_ready;
}

/**
 * @brief Setter de trigger_end
 *
 * @param ultrasound_id ID del transceptor usado para obtenerlo a partir del array de sensores
 */

void port_ultrasound_set_trigger_end(uint32_t ultrasound_id, bool trigger_end){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id); 
    p_ultrasound->trigger_end = trigger_end;
}

/**
 * @brief Esta función configura el temporizador como captura de entrada para medir la duración de la señal de echo
 *
 * @param ultrasound_id ID del transceptor usado para obtenerlo a partir del array de sensores
 */


/*
                                        ECHO
*/


 static void _timer_echo_setup(uint32_t ultrasound_id){

    // 1- Habilitamos el TIMER2, que controla la señal de echo. El TIMER está conectado al APB1
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; 

    // 2- Establecemos el valor del PSC (1MHZ) y del ARR (Max)
    TIM2->PSC = 15;
    TIM2->ARR = 65535;

    /*3- Configuramos el ARPE para que cuando el TIMER llegue al máximo, se reinicie automáticamente
        Además, activa el UG del registro EGR
    */
    TIM2->CR1 = TIM_CR1_ARPE;
    TIM2->EGR = TIM_EGR_UG;

    /* 4- Establecemos la dirección como entrada en el registro de modo de captura/comparación (CCMRx)
    * x es 1 para los canales 1 y 2, y 2 para los canales 3 y 4
    */
    TIM2->CCMR1 &= ~ TIM_CCMR1_CC2S;
    TIM2->CCMR1 |= (0x1 << TIM_CCMR1_CC2S_Pos);

    // 5- Ponemos a 0 los bits ICxF del registro CCMRx (Filtrado digital)
    TIM2->CCMR1 &= ~ TIM_CCMR1_IC2F;

    // 6- Seleccionamos el flanco de activación en CCER estableciendo los valores de los bits CCxNP y CCxP
    TIM2->CCER |= (1 << TIM_CCER_CC2NP_Pos | 1 << TIM_CCER_CC2P_Pos);

    // 7- Programamos el prescaler para capturar cada transición que sea válida. Bit ICxPSC de CCMRx
    TIM2->CCMR1 &= ~ TIM_CCMR1_IC2PSC; 

    // 8- Activamos la Captura de entrada/Comparación a la salida, en el registro CCER de canal correspondiente
    TIM2->CCER |= TIM_CCER_CC2E;

    // 9- Activamos la Captura de entrada/Comparación a la salida, de interrupciones en el registro DIER para el canal correspondiente modificando el bit CCxIE
    TIM2->DIER |= TIM_DIER_CC2IE;

    // 10- Habilitamos el bit de actualización de interrupciones (UIE) modificando el registro DIER
    TIM2->DIER |= TIM_DIER_UIE;

    // 11- Establecemos la prioridad del TIMER2. Prioridad 3, subprioridad 0
    NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 3, 0));
    
}

/**
 * @brief Para el temporizador que controla el echo cuando el echo ha sido recibido
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 */

void port_ultrasound_stop_echo_timer(uint32_t ultrasound_id){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if(p_ultrasound->echo_received){
        TIM2->CR1 &= ~ TIM_CR1_CEN;
        NVIC_DisableIRQ(TIM2_IRQn);
    }
}

/**
 * @brief Resetea los ticks del echo cuando la distancia ya ha sido calculada
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 */

void port_ultrasound_reset_echo_ticks(uint32_t ultrasound_id){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->echo_received = false;
    p_ultrasound->echo_overflows = 0;
    p_ultrasound->echo_init_tick = 0;
    p_ultrasound->echo_end_tick = 0;
}

/**
 * @brief Obtiene el instante de tiempo cuando se recibe (init) el echo
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 * 
 * @return Devuelve el instante de tiempo
 */

uint32_t port_ultrasound_get_echo_init_tick(uint32_t ultrasound_id){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    return p_ultrasound->echo_init_tick;
}

/**
 * @brief Establece el momento en el que llega el echo 
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 * @param echo_init_tick Instante de tiempo a establecer
 */

void port_ultrasound_set_echo_init_tick(uint32_t ultrasound_id, uint32_t echo_init_tick){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->echo_init_tick = echo_init_tick;
}

/**
 * @brief Obtiene el instante de tiempo cuando termina (end) el echo 
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 * 
 * @return Devuelve el instante de tiempo
 */

uint32_t port_ultrasound_get_echo_end_tick(uint32_t ultrasound_id){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    return p_ultrasound->echo_end_tick;
}

/**
 * @brief Establece el momento en el que acaba el echo 
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 * @param echo_end_tick Instante de tiempo a establecer
 */

 void port_ultrasound_set_echo_end_tick(uint32_t ultrasound_id, uint32_t echo_end_tick){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->echo_end_tick = echo_end_tick;
}

/**
 * @brief Devuelve el estado de la señal echo
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 * 
 * @return true: sí se ha recibido el echo 
 * @return false: no se ha recibido el echo
 */

bool port_ultrasound_get_echo_received(uint32_t ultrasound_id){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    return p_ultrasound->echo_received;
}

/**
 * @brief Establece el estado de la señal echo
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 * @param echo_received Estado de la señal de echo a establecer
 */

void port_ultrasound_set_echo_received(uint32_t ultrasound_id, bool echo_received){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->echo_received = echo_received;
}

/**
 * @brief Obtiene el número de desbordamientos del temporizador que controla la señal echo
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 * 
 * @return Número de desbordamientos 
 */

uint32_t port_ultrasound_get_echo_overflows(uint32_t ultrasound_id){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    return p_ultrasound->echo_overflows;
}

/**
 * @brief Establece el número de desbordamientos del temporizador que controla la señal echo
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 * @param echo_overflows Número de desbordamientos
 */

void port_ultrasound_set_echo_overflows(uint32_t ultrasound_id, uint32_t echo_overflows){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->echo_overflows = echo_overflows;
}

                        //TIMEOUT

/**
 * @brief Configuramos el temporizador para generar una interrupción que controle la duración de la medición
 * La duración de la medición viene dada por PORT_PARKING_SENSOR_TIMEOUT_MS
 */

void _timer_new_measurement_setup(){

    // 1- Habilitamos el TIMER5, que controla la señal de timeout. El TIMER está conectado al APB1
    RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;                 
    
    // 2- Apagamos el contador del temporizador para configurarlo nosotros
    TIM5->CR1 &= TIM_CR1_CEN;
    
    // 3- Configuramos el ARPE para que cuando el TIMER llegue al máximo, se reinicie automáticamente
    TIM5->CR1 |= TIM_CR1_ARPE;
    
    // 4- Ponemos el contador de TIMER a 0 para empezar a contar desde 0
    TIM5->CNT = 0;
    
    // 5- Calculamos los valores del prescalado (PSC) y el registro de autorecarga (ARR) para establecer la duración del trigger

    /* 6A- Garantizamos que las siguientes operaciones se hacen con decimales en vez de con enteros
           Establecemos el valor de ARR, 0xFFFF --> 65535    
    */ 
    double doubleSystemCoreClock = (double)SystemCoreClock;
    double ARR = 65535;     
    double timeoutSec = (double)(PORT_PARKING_SENSOR_TIMEOUT_MS)/1000;

    // 6B- Establecemos un valor inicial para el prescalado y lo asignamos al timer
    double prescaler = round((doubleSystemCoreClock * timeoutSec)/(ARR + 1) - 1);       //¡¡Fórmula SDG1!!
    TIM5->PSC = prescaler;

    // 6C- Recalculamos el valor del ARR que tendremos ahora que hemos establecido el PSC
    double ARRnew = round((doubleSystemCoreClock * timeoutSec)/(prescaler + 1) - 1);
    TIM5->ARR = ARRnew;

    /*6D- Comprobamos si el valor de ARR es mayor a 65535
          Si es así, aumentamos el valor de PSC en 1 y recalculamos el ARR
          Si no es así, sirve el valor de ARR calculado anteriormente  
    */ 
    if(ARRnew > 65535){
        prescaler++;
        double ARRnew = round((doubleSystemCoreClock * timeoutSec)/(prescaler + 1) - 1);
        TIM5->PSC = prescaler;
        TIM5->ARR = ARRnew;
    }

    // 7- Aunque los valores de PSC y ARR ya están configurados en sus registros, necesitamos actualizar para que se guarden bien
    TIM5->EGR |= TIM_EGR_UG;

    // 8- Borramos el bit UIF del registro SR por si pudiera causar una interrupción indeseada
    TIM5->SR = ~ TIM_SR_UIF;

    // 9- Habilitamos las interrupciones del TIMER5 estableciendo el bit UIE del registro DIER
    TIM5->DIER |= TIM_DIER_UIE;

    // 10- Establecemos la prioridad del TIMER5. Prioridad 5, subprioridad 0
    NVIC_SetPriority(TIM5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));

}

/**
 * @brief Prepara el timer del trigger y del echo y activa el timer que controla la nueva medida
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 */

void port_ultrasound_start_measurement(uint32_t ultrasound_id){     
    
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    
    //Reseteamos el trigger_ready para indicar que ha comenzado una nueva medición
    p_ultrasound->trigger_ready = false;

    //Reiniciamos los contadores de los distintos temporizadores
    TIM2->CNT = 0;
    TIM3->CNT = 0;          
    TIM5->CNT = 0;

    //Establecemos el trigger_pin a 1
    stm32f4_system_gpio_write(p_ultrasound->p_trigger_port, p_ultrasound->trigger_pin, true);

    //Activamos las interrupciones de los temporizadores
    NVIC_EnableIRQ(TIM3_IRQn);
    NVIC_EnableIRQ(TIM2_IRQn);
    NVIC_EnableIRQ(TIM5_IRQn);

    //Activamos los timers
    TIM2->CR1 |= TIM_CR1_CEN; 
    TIM3->CR1 |= TIM_CR1_CEN;       
    TIM5->CR1 |= TIM_CR1_CEN;
}

/**
 * @brief Inicia el timer que controla la nueva medición
 */

void port_ultrasound_start_new_measurement_timer(){
    NVIC_EnableIRQ(TIM5_IRQn);
    TIM5->CR1 |= TIM_CR1_CEN;
}

/**
 * @brief Detiene el timer que controla la nueva medición
 */

void port_ultrasound_stop_new_measurement_timer(){
    TIM5->CR1 &= ~TIM_CR1_CEN;
    NVIC_DisableIRQ(TIM5_IRQn);
}

/**
 * @brief Para todos los timers y reinicia el echo_ticks
 * 
 * @param ultrasound_id Se usa como índice para seleccionar el elemento de ultrasound_arr[]
 */

void port_ultrasound_stop_ultrasound(uint32_t ultrasound_id){
    port_ultrasound_stop_trigger_timer(ultrasound_id);
    port_ultrasound_stop_echo_timer(ultrasound_id);
    port_ultrasound_stop_new_measurement_timer();
    port_ultrasound_reset_echo_ticks(ultrasound_id);
}	

                //PORT_ULTRASOUND_INIT

void port_ultrasound_init(uint32_t ultrasound_id)
{
    /* Get the ultrasound sensor */
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);

    //Inicialización de los echos
    p_ultrasound->echo_init_tick = 0;
    p_ultrasound->echo_end_tick = 0;
    p_ultrasound->echo_overflows = 0;
    
    //Inicialización de los flags
    p_ultrasound->trigger_ready = true;
    p_ultrasound->trigger_end = false;
    p_ultrasound->echo_received = false;

    //Configuramos la GPIO del trigger en modo salida sin pull up ni pull down
    stm32f4_system_gpio_config(p_ultrasound->p_trigger_port, p_ultrasound->trigger_pin, STM32F4_GPIO_MODE_OUT, STM32F4_GPIO_PUPDR_NOPULL);

    //Configuramos la GPIO del echo en modo alterno sin pull up ni pull down
    stm32f4_system_gpio_config(p_ultrasound->p_echo_port, p_ultrasound->echo_pin, STM32F4_GPIO_MODE_AF, STM32F4_GPIO_PUPDR_NOPULL);    

    //Llamada a la función del trigger
    _timer_trigger_setup();

    //Llamada a la función del echo
    _timer_echo_setup(ultrasound_id);
    
    //Configuramos la función alterna del echo pin
    stm32f4_system_gpio_config_alternate(p_ultrasound->p_echo_port, p_ultrasound->echo_pin, p_ultrasound->echo_alt_fun);
    
    //Llamamos a la función para realizar la nueva medida
    _timer_new_measurement_setup();

}

// Util
void stm32f4_ultrasound_set_new_trigger_gpio(uint32_t ultrasound_id, GPIO_TypeDef *p_port, uint8_t pin)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->p_trigger_port = p_port;
    p_ultrasound->trigger_pin = pin;
}

void stm32f4_ultrasound_set_new_echo_gpio(uint32_t ultrasound_id, GPIO_TypeDef *p_port, uint8_t pin)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->p_echo_port = p_port;
    p_ultrasound->echo_pin = pin;
}