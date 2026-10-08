/**
 * @file stm32f4_system.c
 * @brief This file implements port layer for the system functions in the STM32F4 platform.
 * @author SDG2. Román Cárdenas (r.cardenas@upm.es) and Josué Pagán (j.pagan@upm.es)
 * @date 2025-01-01
 */

/* HW dependent includes */
#include "port_system.h"
#include "stm32f4_system.h"

#ifdef USE_SEMIHOSTING
extern void initialise_monitor_handles(void);
#endif

//------------------------------------------------------
// FILE-SPECIFIC DEFINITIONS
//------------------------------------------------------
#define HSI_VALUE ((uint32_t)16000000) /*!< Value of the Internal oscillator in Hz */
/* Timer configuration */
#define RCC_HSI_CALIBRATION_DEFAULT 0x10U            /*!< Default HSI calibration trimming value */
#define TICK_FREQ_1KHZ 1U                            /*!< Frequency in kHz of the System tick */
#define NVIC_PRIORITY_GROUP_0 ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority, \
                                                         4 bits for subpriority */
#define NVIC_PRIORITY_GROUP_4 ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority, \
                                                         0 bit  for subpriority */
/* Power */
#define POWER_REGULATOR_VOLTAGE_SCALE3 0x01 /*!< Scale 3 mode: the maximum value of fHCLK is 120 MHz. */

//------------------------------------------------------
// PRIVATE (STATIC) VARIABLES
//------------------------------------------------------
static volatile uint32_t msTicks = 0; /*!< Variable to store millisecond ticks. @warning **It must be declared volatile!** Just because it is modified in an ISR. **Add it to the definition** after *static*. */

//------------------------------------------------------
// PUBLIC (GLOBAL) VARIABLES
//------------------------------------------------------

/* These variables are declared extern in CMSIS (system_stm32f4xx.h) */
uint32_t SystemCoreClock = HSI_VALUE;                                               /*!< Frequency of the System clock */
const uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9}; /*!< Prescaler values for AHB bus */
const uint8_t APBPrescTable[8] = {0, 0, 0, 0, 1, 2, 3, 4};                          /*!< Prescaler values for APB bus */

//------------------------------------------------------
// PRIVATE (STATIC) FUNCTIONS
//------------------------------------------------------

/**
 * @brief System Clock Configuration
 *
 * @attention This function should NOT be accesible from the outside to avoid configuration problems.
 * @note This function starts a system timer that generates a SysTick every 1 ms.
 */
static void system_clock_config(void)
{
  /** Configure the main internal regulator output voltage */
  /* Power controller (PWR) */
  /* Control the main internal voltage regulator output voltage to achieve a trade-off between performance and power consumption when the device does not operate at the maximum frequency */
  PWR->CR &= ~PWR_CR_VOS; // Clean and set value
  PWR->CR |= (PWR_CR_VOS & (POWER_REGULATOR_VOLTAGE_SCALE3 << PWR_CR_VOS_Pos));

  /* Initializes the RCC Oscillators. */
  /* Adjusts the Internal High Speed oscillator (HSI) calibration value.*/
  RCC->CR &= ~RCC_CR_HSITRIM; // Clean and set value
  RCC->CR |= (RCC_CR_HSITRIM & (RCC_HSI_CALIBRATION_DEFAULT << RCC_CR_HSITRIM_Pos));

  /* RCC Clock Config */
  /* Initializes the CPU, AHB and APB buses clocks */
  /* To correctly read data from FLASH memory, the number of wait states (LATENCY)
      must be correctly programmed according to the frequency of the CPU clock
      (HCLK) and the supply voltage of the device. */

  /* Increasing the number of wait states because of higher CPU frequency */
  FLASH->ACR = FLASH_ACR_LATENCY_2WS; /* Program the new number of wait states to the LATENCY bits in the FLASH_ACR register */

  /* Change in clock source is performed in 16 clock cycles after writing to CFGR */
  RCC->CFGR &= ~RCC_CFGR_SW; // Clean and set value
  RCC->CFGR |= (RCC_CFGR_SW & (RCC_CFGR_SW_HSI << RCC_CFGR_SW_Pos));

  /* Update the SystemCoreClock global variable */
  SystemCoreClock = HSI_VALUE >> AHBPrescTable[(RCC->CFGR & RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos];

  /* Configure the source of time base considering new system clocks settings */
  SysTick_Config(SystemCoreClock / (1000U / TICK_FREQ_1KHZ)); /* Set Systick to 1 ms */
}

//------------------------------------------------------
// PUBLIC (GLOBAL) FUNCTIONS
//------------------------------------------------------

// ------------------------------------------------------
// Implementation of PORT system functions that are called from the platform-independent code.
// i.e., the following functions do not depend on the platform and are declared in the
// port_system.h file.
// ------------------------------------------------------
/**
 * @brief  Setup the microcontroller system
 *         Initialize the FPU setting, vector table location and External memory
 *         configuration.
 *
 * @note   This function is called at startup by CMSIS in startup_stm32f446xx.s.
 */
void SystemInit(void)
{
/* FPU settings ------------------------------------------------------------*/
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
  SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2)); /* set CP10 and CP11 Full Access */
#endif

#if defined(DATA_IN_ExtSRAM) || defined(DATA_IN_ExtSDRAM)
  SystemInit_ExtMemCtl();
#endif /* DATA_IN_ExtSRAM || DATA_IN_ExtSDRAM */

  /* Configure the Vector Table location -------------------------------------*/
#if defined(USER_VECT_TAB_ADDRESS)
  SCB->VTOR = VECT_TAB_BASE_ADDRESS | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM */
#endif                                                 /* USER_VECT_TAB_ADDRESS */
}

uint32_t port_system_init()
{

#ifdef USE_SEMIHOSTING
  initialise_monitor_handles();
#endif

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  /* Configure Flash prefetch, Instruction cache, Data cache */
  /* Instruction cache enable */
  FLASH->ACR |= FLASH_ACR_ICEN;

  /* Data cache enable */
  FLASH->ACR |= FLASH_ACR_DCEN;

  /* Prefetch cache enable */
  FLASH->ACR |= FLASH_ACR_PRFTEN;

  /* Set Interrupt Group Priority */
  NVIC_SetPriorityGrouping(NVIC_PRIORITY_GROUP_4);

  /* Use systick as time base source and configure 1ms tick (default clock after Reset is HSI) */
  /* Configure the SysTick IRQ priority. It must be the highest (lower number: 0)*/
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0U, 0U)); /* Tick interrupt priority */

  /* Init the low level hardware */
  /* Reset and clock control (RCC) */
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; /* Syscfg clock enabling */

  /* Peripheral clock enable register */
  RCC->APB1ENR |= RCC_APB1ENR_PWREN; /* PWREN: Power interface clock enable */

  /* Configure the system clock */
  system_clock_config();

  return 0;
}

//------------------------------------------------------
// TIMER RELATED FUNCTIONS
//------------------------------------------------------
void port_system_delay_ms(uint32_t ms)
{
  uint32_t tickstart = port_system_get_millis();

  while ((port_system_get_millis() - tickstart) < ms)
  {
  }
}

void port_system_delay_until_ms(uint32_t *p_t, uint32_t ms)
{
  uint32_t until = *p_t + ms;
  uint32_t now = port_system_get_millis();
  if (until > now)
  {
    port_system_delay_ms(until - now);
  }
  *p_t = port_system_get_millis();
}

uint32_t port_system_get_millis()
{
  return msTicks;
}

void port_system_set_millis(uint32_t ms)
{
  msTicks = ms;
}

void port_system_systick_suspend(){
  SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
}

void port_system_systick_resume(){
  SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
}

// ------------------------------------------------------
// Implementation of PORT system functions that are called from the platform-dependent code.
// i.e., the following functions do depend on the platform and are declared in the
// stm32f4_system.h file.
// ------------------------------------------------------
//------------------------------------------------------
// GPIO RELATED FUNCTIONS
//------------------------------------------------------
void stm32f4_system_gpio_config(GPIO_TypeDef *p_port, uint8_t pin, uint8_t mode, uint8_t pupd)
{
  if (p_port == GPIOA)
  {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; /* GPIOA_CLK_ENABLE */
  }
  else if (p_port == GPIOB)
  {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; /* GPIOB_CLK_ENABLE */
  }
  else if (p_port == GPIOC)
  {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; /* GPIOC_CLK_ENABLE */
  }

  /* Clean ( &=~ ) by displacing the base register and set the configuration ( |= ) */
  p_port->MODER &= ~(GPIO_MODER_MODER0 << (pin * 2U));
  p_port->MODER |= (mode << (pin * 2U));

  p_port->PUPDR &= ~(GPIO_PUPDR_PUPD0 << (pin * 2U));
  p_port->PUPDR |= (pupd << (pin * 2U));
}

void stm32f4_system_gpio_config_exti(GPIO_TypeDef *p_port, uint8_t pin, uint32_t mode)
{
  uint32_t port_selector = 0;

  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

  /* SYSCFG external interrupt configuration register */
  if (p_port == GPIOA)
  {
    port_selector = 0;
  }
  else if (p_port == GPIOB)
  {
    port_selector = 1;
  }
  else if (p_port == GPIOC)
  {
    port_selector = 2;
  }

  uint32_t base_mask = 0x0FU;
  uint32_t displacement = (pin % 4) * 4;

  SYSCFG->EXTICR[pin / 4] &= ~(base_mask << displacement);
  SYSCFG->EXTICR[pin / 4] |= (port_selector << displacement);

  /* Rising trigger selection register (EXTI_RTSR) */
  EXTI->RTSR &= ~BIT_POS_TO_MASK(pin);
  if (mode & STM32F4_TRIGGER_RISING_EDGE)
  {
    EXTI->RTSR |= BIT_POS_TO_MASK(pin);
  }

  /* Falling trigger selection register (EXTI_FTSR) */
  EXTI->FTSR &= ~BIT_POS_TO_MASK(pin);
  if (mode & STM32F4_TRIGGER_FALLING_EDGE)
  {
    EXTI->FTSR |= BIT_POS_TO_MASK(pin);
  }

  /* Event mask register (EXTI_EMR) */
  EXTI->EMR &= ~BIT_POS_TO_MASK(pin);
  if (mode & STM32F4_TRIGGER_ENABLE_EVENT_REQ)
  {
    EXTI->EMR |= BIT_POS_TO_MASK(pin);
  }

  /* Clear EXTI line configuration */
  EXTI->IMR &= ~BIT_POS_TO_MASK(pin);

  /* Interrupt mask register (EXTI_IMR) */
  if (mode & STM32F4_TRIGGER_ENABLE_INTERR_REQ)
  {
    EXTI->IMR |= BIT_POS_TO_MASK(pin);
  }
}

void stm32f4_system_gpio_exti_enable(uint8_t pin, uint8_t priority, uint8_t subpriority)
{
  NVIC_SetPriority(GET_PIN_IRQN(pin), NVIC_EncodePriority(NVIC_GetPriorityGrouping(), priority, subpriority));
  NVIC_EnableIRQ(GET_PIN_IRQN(pin));
}

void stm32f4_system_gpio_exti_disable(uint8_t pin)
{
  NVIC_DisableIRQ(GET_PIN_IRQN(pin));
}

void stm32f4_system_gpio_config_alternate(GPIO_TypeDef *p_port, uint8_t pin, uint8_t alternate)
{
  uint32_t base_mask = 0x0FU;
  uint32_t displacement = (pin % 8) * 4;

  p_port->AFR[(uint8_t)(pin / 8)] &= ~(base_mask << displacement);
  p_port->AFR[(uint8_t)(pin / 8)] |= (alternate << displacement);
}

// ------------------------------------------------------
// POWER RELATED FUNCTIONS
// ------------------------------------------------------

bool stm32f4_system_gpio_read (GPIO_TypeDef *p_port, uint8_t pin){
  bool value=(bool)(p_port->IDR & BIT_POS_TO_MASK(pin));
  return value;
}

void  stm32f4_system_gpio_write (GPIO_TypeDef *p_port, uint8_t pin, bool value){
  if(value==true){
    p_port->BSRR=BIT_POS_TO_MASK(pin);
  }
  else{
    p_port->BSRR=BIT_POS_TO_MASK(pin) << 16;
  }
}

void stm32f4_system_gpio_toggle (GPIO_TypeDef *p_port, uint8_t pin){
  bool resultadoLectura= stm32f4_system_gpio_read(p_port, pin);
  stm32f4_system_gpio_write(p_port,pin,!resultadoLectura);
}

void port_system_power_stop(){
  MODIFY_REG(PWR->CR, (PWR_CR_PDDS | PWR_CR_LPDS), PWR_CR_LPDS);   // Select the regulator state in Stop mode: Set PDDS and LPDS bits according to PWR_Regulator value
  SCB->SCR |= ((uint32_t)SCB_SCR_SLEEPDEEP_Msk);   // Set SLEEPDEEP bit of Cortex System Control Register
  __WFI(); // Select Stop mode entry : Request Wait For Interrupt
  SCB->SCR &= ~((uint32_t)SCB_SCR_SLEEPDEEP_Msk); // Reset SLEEPDEEP bit of Cortex System Control Register
}

void port_system_power_sleep(){
  MODIFY_REG(PWR->CR, (PWR_CR_PDDS | PWR_CR_LPDS), PWR_CR_LPDS);   // Select the regulator state in Stop mode: Set PDDS and LPDS bits according to PWR_Regulator value
  SCB->SCR &= ~((uint32_t)SCB_SCR_SLEEPDEEP_Msk);   // Reset SLEEPDEEP bit of Cortex System Control Register
  __WFI(); // Select Sleep mode entry : Request Wait For Interrupt
}

void port_system_sleep(void){
  port_system_systick_suspend();
  port_system_power_sleep();
}
