/**
 * @file stm32f4_ultrasound.h
 * @brief Header for stm32f4_ultrasound.c file.
 * @author Nicolás Henández Martín
 * @author Gonzalo Leis Varela
 * @date 17/03/2025
 */
#ifndef STM32F4_ULTRASOUND_H_
#define STM32F4_ULTRASOUND_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */

/* HW dependent includes */

/* Defines and enums ----------------------------------------------------------*/
/* Defines */
#define STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO GPIOB  //GPIO asociada a la señal Trigger
#define STM32F4_REAR_PARKING_SENSOR_TRIGGER_PIN 0   //Pin asociado a la señal Trigger
#define STM32F4_REAR_PARKING_SENSOR_ECHO_GPIO GPIOA //GPIO asociada a la señal Echo
#define STM32F4_REAR_PARKING_SENSOR_ECHO_PIN 1  //GPIO asociada a la señal Echo
/* Function prototypes and explanation -------------------------------------------------*/
/**
 * @brief Auxiliary function to change the GPIO and pin of the trigger pin of an ultrasound transceiver. This function is used for testing purposes mainly although it can be used in the final implementation if needed.
 *
 * @param ultrasound_id ID of the trigger signal to change.
 * @param p_port New GPIO port for the trigger signal.
 * @param pin New GPIO pin for the trigger signal.
 *
 */
void stm32f4_ultrasound_set_new_trigger_gpio(uint32_t ultrasound_id, GPIO_TypeDef *p_port, uint8_t pin);

/**
 * @brief Auxiliary function to change the GPIO and pin of the echo pin of an ultrasound transceiver. This function is used for testing purposes mainly although it can be used in the final implementation if needed.
 *
 * @param ultrasound_id ID of the echo signal to change.
 * @param p_port New GPIO port for the echo signal.
 * @param pin New GPIO pin for the echo signal.
 *
 */
void stm32f4_ultrasound_set_new_echo_gpio(uint32_t ultrasound_id, GPIO_TypeDef *p_port, uint8_t pin);


#endif /* STM32F4_ULTRASOUND_H_ */
