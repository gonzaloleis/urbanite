/**
 * @file port_display.h
 * @brief Header for the portable functions to interact with the HW of the display system. The functions must be implemented in the platform-specific code.
 * @author Nicolás Hernández Martín
 * @author Gonzalo Leis Varela
 * @date 08/04/2025
 */
#ifndef PORT_DISPLAY_SYSTEM_H_
#define PORT_DISPLAY_SYSTEM_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>

/* Typedefs --------------------------------------------------------------------*/
typedef struct{
    
    uint8_t r;
    uint8_t g;              //Estructura para definir un color según el esquema RGB
    uint8_t b;

} rgb_color_t;

/* Defines and enums ----------------------------------------------------------*/

#define PORT_REAR_PARKING_DISPLAY_ID  0                     // ID del display
#define COLOR_OFF (rgb_color_t){0, 0, 0}                    // Combinación de colores para APAGADO
#define COLOR_RED (rgb_color_t){255, 0, 0}                  // Color Rojo
#define COLOR_ORANGE (rgb_color_t){255, 7, 0}               // Color Naranja
#define COLOR_YELLOW (rgb_color_t){255, 12, 0}              // Color Amarillo
#define COLOR_LIGHT_GREEN (rgb_color_t){128, 94, 0}         // Color Verde Claro 
#define COLOR_GREEN (rgb_color_t){0, 255, 0}                // Color Verde  
#define COLOR_CYAN (rgb_color_t){0, 255, 128}               // Color Verde Azulado
#define COLOR_LIGHT_BLUE (rgb_color_t){0, 255, 255}         // Color Azul Claro
#define COLOR_BLUE (rgb_color_t){0, 128, 255}               // Color Azul 
#define COLOR_DARK_BLUE (rgb_color_t){0, 0, 255}            // Color Azul Oscuro
#define COLOR_PURPLE (rgb_color_t){128, 0, 255}             // Color Morado

/* Defines */

/* Function prototypes and explanation -------------------------------------------------*/

/**
 * @brief Configura el display con las especificaciones HW
 *
 * @param display_id Índice usado para seleccionar un elemento de displays_arr[]
 */

void port_display_init(uint32_t display_id);

/**
 * @brief Deshabilita el temporizador asociado a el LED RGB, establece el registro de Captura/Comparación de cada canal del LED
 *  y habilita de nuevo el temporizador
 *
 * @param display_id Índice usado para seleccionar un elemento de displays_arr[]
 */

void port_display_set_rgb(uint32_t display_id, rgb_color_t color);

#endif /* PORT_DISPLAY_SYSTEM_H_ */