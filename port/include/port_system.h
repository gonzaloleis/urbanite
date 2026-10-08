/**
 * @file port_system.h
 * @brief Header for port_system.c file.
 * @author SDG2. Román Cárdenas (r.cardenas@upm.es) and Josué Pagán (j.pagan@upm.es)
 * @date 2025-01-01
 */

#ifndef PORT_SYSTEM_H_
#define PORT_SYSTEM_H_

/* Includes del sistema */
#include <stdint.h>

/**
 * @brief Initializes the system.
 */
uint32_t port_system_init(void);

/**
 * @brief Returns the number of milliseconds since the system started.
 *
 * @retval number of milliseconds since the system started.
 */
uint32_t port_system_get_millis(void);

/**
 * @brief Sets the number of milliseconds since the system started.
 *
 * @param ms New number of milliseconds since the system started.
 */
void port_system_set_millis(uint32_t ms);

/**
 * @brief Delays the program execution for the specified number of milliseconds.
 *
 * @param ms Number of milliseconds to delay.
 */
void port_system_delay_ms(uint32_t ms);

/**
 * @brief Delays the program execution until the specified number of milliseconds since the system started.
 *
 * @param t Pointer to the variable that stores the number of milliseconds to delay until.
 * @param ms Number of milliseconds to delay until.
 *
 * @note This function modifies the value of the variable pointed by t to the number of milliseconds to delay until.
 * @note This function is useful to implement periodic tasks.
 */
void port_system_delay_until_ms(uint32_t *t, uint32_t ms);

/**
 * @brief Establece el sistema en modo stop para el bajo consumo
 */

void port_system_power_stop();

/**
 * @brief Establece el sistema en modo sleep para el bajo consumo
 */

void port_system_power_sleep();

/**
 * @brief Una vez que se llama a esta función, la interrupción SysTick se deshabilitará para ahorrar más energía y no generar ninguna interrupción.
 */

void port_system_systick_suspend();

/**
 * @brief Una vez que se llama a esta función, se habilitará la interrupción SysTick y se reanudará el incremento de Tick.
 */

void port_system_systick_resume();

/**
 * @brief Se activa el modo bajo consumo para el modo sleep 
 */

void port_system_sleep();

#endif /* PORT_SYSTEM_H_ */
