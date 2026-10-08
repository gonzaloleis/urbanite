/**
 * @file stm32f4_display.h
 * @brief Header for stm32f4_display.c file.
 * @author Nicolás Hernández Martín
 * @author GOnzalo Leis Varela
 * @date 08/04/2025
 */
#ifndef STM32F4_DISPLAY_SYSTEM_H_
#define STM32F4_DISPLAY_SYSTEM_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>
#include "stm32f4xx.h"
/* HW dependent includes */

/* Defines and enums ----------------------------------------------------------*/
#define STM32F4_REAR_PARKING_DISPLAY_RGB_R_GPIO GPIOB       //GPIO asociada al led RGB
#define STM32F4_REAR_PARKING_DISPLAY_RGB_R_PIN  6           //PIN asociado al ROJO
#define STM32F4_REAR_PARKING_DISPLAY_RGB_G_GPIO GPIOB       //GPIO asociada al led RGB
#define STM32F4_REAR_PARKING_DISPLAY_RGB_G_PIN  8           //PIN asociado al VERDE
#define STM32F4_REAR_PARKING_DISPLAY_RGB_B_GPIO GPIOB       //GPIO asociada al led RGB
#define STM32F4_REAR_PARKING_DISPLAY_RGB_B_PIN  9           //PIN asociado al AZUL
/* Defines */

#endif /* STM32F4_DISPLAY_SYSTEM_H_ */