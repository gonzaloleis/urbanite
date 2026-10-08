/**
 * @file stm32f4_system.h
 * @brief Header for stm32f4_system.c file.
 * @author SDG2. Román Cárdenas (r.cardenas@upm.es) and Josué Pagán (j.pagan@upm.es)
 * @date 2025-01-01
 */

 #ifndef STM32F4_SYSTEM_H_
 #define STM32F4_SYSTEM_H_
 
 /* Includes ------------------------------------------------------------------*/
 /* Standard C includes */
 #include <stdbool.h>
 
 /* HW dependent includes */
 #include "stm32f4xx.h"
 
 /* Defines and enums ----------------------------------------------------------*/
 /* Defines */
 #define BIT_POS_TO_MASK(x) (0x01 << (x))                                                                      /*!< Convert the index of a bit into a mask by left shifting */
 #define BASE_MASK_TO_POS(m, p) ((m) << (p))                                                                   /*!< Move a mask defined in the LSBs to upper positions by shifting left p bits */
 #define GET_PIN_IRQN(pin) ((pin) >= 10 ? EXTI15_10_IRQn : ((pin) >= 5 ? EXTI9_5_IRQn : (EXTI0_IRQn + (pin)))) /*!< Compute the IRQ number associated to a GPIO pin */
 
 /* GPIOs */
 #define STM32F4_GPIO_MODE_IN 0x00U  /*!< Input mode */
 #define STM32F4_GPIO_MODE_OUT 0x01U /*!< Output mode */
 #define STM32F4_GPIO_MODE_AF 0x02U  /*!< Alternate function mode */
 #define STM32F4_GPIO_MODE_AN 0x03U  /*!< Analog mode */
 
 #define STM32F4_GPIO_PUPDR_NOPULL 0x00U   /*!< No pull-up, no pull-down */
 #define STM32F4_GPIO_PUPDR_PULLUP 0x01U   /*!< Pull-up */
 #define STM32F4_GPIO_PUPDR_PULLDOWN 0x02U /*!< Pull-down */
 
 /* External interrupts */
 #define STM32F4_TRIGGER_RISING_EDGE 0x01U                                                      /*!< Interrupt mask for detecting rising edge */
 #define STM32F4_TRIGGER_FALLING_EDGE 0x02U                                                     /*!< Interrupt mask for detecting falling edge */
 #define STM32F4_TRIGGER_BOTH_EDGE (STM32F4_TRIGGER_RISING_EDGE | STM32F4_TRIGGER_FALLING_EDGE) /*!< Interrupt mask for detecting both rising and falling edges */
 #define STM32F4_TRIGGER_ENABLE_EVENT_REQ 0x04                                                  /*!< Interrupt mask for enabling event request */
 #define STM32F4_TRIGGER_ENABLE_INTERR_REQ 0x08U                                                /*!< Interrupt mask for enabling interrupt request */
 
 /* Alternate functions */
 #define STM32F4_AF1 0x01U /*!< Alternate function 1 */
 #define STM32F4_AF2 0x02U /*!< Alternate function 2 */
 
 /** @verbatim
       ==============================================================================
                               ##### How to use GPIOs #####
       ==============================================================================
       [..]
         (#) Enable the GPIO AHB clock using the RCC->AHB1ENR register.
 
         (#) Configure the GPIO pin.
             (++) Configure the IO mode.
             (++) Activate Pull-up, Pull-down resistor.
             (++) In case of Output or alternate function mode, configure the speed if needed.
             (++) Configure digital or analog mode.
             (++) In case of external interrupt/event select the type (interrupt or event) and
                  the corresponding trigger event (rising or falling or both).
 
         (#) In case of external interrupt/event mode selection, configure NVIC IRQ priority
             mapped to the EXTI line and enable it using.
 
         (#) To get the level of a pin configured in input mode use the GPIOx_IDR register.
 
         (#) To set/reset the level of a pin configured in output mode use the GPIOx_BSRR register
             to SET (bits 0..15) or RESET (bits 16..31) the GPIO.
 
         @endverbatim
       ******************************************************************************
       */
 /**
  * @brief Configure the mode and pull of a GPIO
  *
  * > 1. Enable GPIOx clock in AHB1ENR \n
  * > 2. Set mode in MODER \n
  * > 3. Set pull up/down configuration
  *
  * @note This function performs the GPIO Port Clock Enable. It may occur that a port clock is re-enabled,
  *       it does not matter if it was already enabled. *
  * @note This function enables the AHB1 peripheral clock. After reset, the peripheral clock (used for registers
  *       read/write access) is disabled and the application software has to enable this clock before using it.
  *
  * @param p_port Port of the GPIO (CMSIS struct like)
  * @param pin Pin/line of the GPIO (index from 0 to 15)
  * @param mode Input, output, alternate, or analog
  * @param pupd Pull-up, pull-down, or no-pull
  *
  * @retval None
  */
 void stm32f4_system_gpio_config(GPIO_TypeDef *p_port, uint8_t pin, uint8_t mode, uint8_t pupd);
 
 /**
  * @brief Configure the alternate function of a GPIO
  *
  * > 1. **Create a 4-bit base mask**. \n
  * > 2. Shift left the mask depending on the value of the given **`pin` modulo 8.** \n
  * > &nbsp;&nbsp;&nbsp;&nbsp;💡 The value of `pin` ranges from 0 to 15. The registers GPIOx_AFRH and GPIOx_AFRL implement 8 groups of 4 bits each. In order to use the value of `pin` as index to select the corresponding group of bits, we can use the remainder of the division by 8. \n
  * > 3. Clean and set the bits **as shown in the tutorial document**. \n
  * > &nbsp;&nbsp;&nbsp;&nbsp;💡 Clean the corresponding bit on element `0` or `1` of the AFR array (*e.g*, `GPIOA->AFR[0]` for GPIOx_AFRL) \n
  * > &nbsp;&nbsp;&nbsp;&nbsp;💡 Set the given value (`alternate`) of the alternate function, using bit shifting, for example. \n
  * \n
  * > 💡 **You can define your own masks for each alternate function (not recommended), or you can use the macro `BASE_MASK_TO_POS(m, p)` to get the mask of a base mask. Example:** \n
  * > &nbsp;&nbsp;&nbsp;&nbsp;A base mask `m` equals `0x03` (`0b 0000 0011` in binary) can be shifted `p` equals `8` positions `BASE_MASK_TO_POS(0x03, 8)` resulting in `0x300` (`0b 0011 0000 0000` in binary). \n
  *
  * @note The AFR register is a 2-element array representing GPIO alternate function high an low registers (GPIOx_AFRH and GPIOx_AFRL) \n
  * AFRLy: Alternate function selection for port x pin y (y = 0..7) \n
  * AFRHy: Alternate function selection for port x pin y (y = 8..15)
  *
  * @param p_port Port of the GPIO (CMSIS struct like)
  * @param pin Pin/line of the GPIO (index from 0 to 15)
  * @param alternate Alternate function number (values from 0 to 15) according to table of the datasheet: "Table 11. Alternate function".
  *
  * @retval None
  */
 void stm32f4_system_gpio_config_alternate(GPIO_TypeDef *p_port, uint8_t pin, uint8_t alternate);
 
 /**
  * @brief Configure the external interruption or event of a GPIO
  *
  * > 1. **Enable the System configuration controller clock (SYSCFG).** Enable the SYSCFG by setting the bit SYSCFGEN of the peripheral clock enable register (RCC_APB2ENR). The system configuration controller is used here to manage the external interrupt line connection to the GPIOs. \n
  * > &nbsp;&nbsp;&nbsp;&nbsp;💡 As usual, you can access to the register (`APB2ENR`) as element of the structure `RCC`. You can use the macro `RCC_APB2ENR_SYSCFGEN` defined in `stm32f446xx.h` to set the bit. Look for the "RCC_APB2ENR" register in the Reference Manual if you need more information. \n
  * > \n
  * > 2. **Associate the external interruption line to the given port.** Clean and set the bits **as shown in the tutorial document**. \n
  * > &nbsp;&nbsp;&nbsp;&nbsp;💡 Depending on the pin number, use the register SYSCFG_EXTICR1, SYSCFG_EXTICR2, SYSCFG_EXTICR3, or SYSCFG_EXTICR4. The structure `SYSCFG` contains a 4-element array called `EXTICR`; the first element (`EXTICR[0]`) configures the register SYSCFG_EXTICR1, and so on. \n
  * > &nbsp;&nbsp;&nbsp;&nbsp;💡 To clean the EXTIx bits, you can create a mask depending on the `pin` value.   \n
  * > &nbsp;&nbsp;&nbsp;&nbsp;💡 To associate the external interruption to the given port, *i.e.* to set the EXTIx bits, you can create another mask depending on the `port` value.   \n
  * > \n
  * > 3. **Select the direction of the trigger**: rising edge, falling edge, or both, depending on the value of the given `mode`.  \n
  * > &nbsp;&nbsp;&nbsp;&nbsp;💡 If *rising edge*: activate the corresponding bit on the EXTI_RTSR register (element `RTSR`) of the `EXTI` structure. \n
  * > &nbsp;&nbsp;&nbsp;&nbsp;💡 If *falling edge*: activate the corresponding bit on the EXTI_FTSR register (element `FTSR`) of the `EXTI` structure. \n
  * > &nbsp;&nbsp;&nbsp;&nbsp;💡 If *both*: activate the corresponding bit on both registers. \n
  * > \n
  * > 4. **Select the interrupt and/or event request**: depending on the  value of the given `mode`.  \n
  * > &nbsp;&nbsp;&nbsp;&nbsp;💡 If *event request* enable: activate the corresponding bit on the EXTI_EMR register (element `EMR`) of the `EXTI` structure. \n
  * > &nbsp;&nbsp;&nbsp;&nbsp;💡 If *interrupt request* enable: activate the corresponding bit on the EXTI_IMR register (element `IMR`) of the `EXTI` structure. \n
  * \n
  * > 💡 **You can define your own masks for each pin value (not recommended), or you can use the `BIT_POS_TO_MASK(pin)` macro to get the mask of a pin.**
  *
  * @warning It is highly recommended to clean the corresponding bit of each register (`RSTR`, `FTSR`, `EMR`, `IMR`) before activating it.
  *
  * @param p_port Port of the GPIO (CMSIS struct like)
  * @param pin Pin/line of the GPIO (index from 0 to 15)
  * @param mode Trigger mode can be a combination (OR) of: (i) direction: rising edge (0x01), falling edge (0x02), (ii)  event request (0x04), or (iii) interrupt request (0x08).
  * @retval None
  */
 void stm32f4_system_gpio_config_exti(GPIO_TypeDef *p_port, uint8_t pin, uint32_t mode);
 
 /**
  * @brief Enable interrupts of a GPIO line (pin)
  *
  * @param pin Pin/line of the GPIO (index from 0 to 15)
  * @param priority Priority level (from highest priority: 0, to lowest priority: 15)
  * @param subpriority Subpriority level (from highest priority: 0, to lowest priority: 15)
  *
  * @retval None
  */
 void stm32f4_system_gpio_exti_enable(uint8_t pin, uint8_t priority, uint8_t subpriority);
 
 /**
  * @brief Disable interrupts of a GPIO line (pin)
  *
  * @param pin Pin/line of the GPIO (index from 0 to 15)
  *
  * @retval None
  */
 void stm32f4_system_gpio_exti_disable(uint8_t pin);
 
 bool stm32f4_system_gpio_read (GPIO_TypeDef *p_port, uint8_t pin);

 void stm32f4_system_gpio_write (GPIO_TypeDef *p_port, uint8_t pin, bool value);
 
 void stm32f4_system_gpio_toggle (GPIO_TypeDef *p_port, uint8_t pin);

 #endif /* STM32F4_SYSTEM_H_ */