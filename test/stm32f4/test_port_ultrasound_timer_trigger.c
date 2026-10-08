/**
 * @file test_port_ultrasound.c
 * @brief Unit test for the ultrasound port driver.
 *
 * It checks the configuration of the ultrasound transceiver peripheral and the GPIO pins and other configurations using the Unity framework.
 *
 * @author Sistemas Digitales II
 * @date 2025-01-01
 */

/* Includes ------------------------------------------------------------------*/
/* HW independent libraries */
#include <stdlib.h>
#include <unity.h>

/* HW dependent libraries */
#include "port_ultrasound.h"
#include "port_system.h"
#include "stm32f4_system.h"
#include "stm32f4_ultrasound.h"
#include "stm32f4xx.h"

/* Defines and enums ----------------------------------------------------------*/
#define TEST_PORT_REAR_PARKING_SENSOR_ID 0 /*!< Ultrasound identifier @hideinitializer */

// Trigger timer configuration
#define REAR_TRIGGER_TIMER TIM3                            /*!< Trigger signal timer @hideinitializer */
#define REAR_TRIGGER_TIMER_IRQ TIM3_IRQn                   /*!< Trigger signal timer IRQ @hideinitializer */
#define REAR_TRIGGER_TIMER_IRQ_PRIO 4                      /*!< Trigger signal timer IRQ priority @hideinitializer */
#define REAR_TRIGGER_TIMER_IRQ_SUBPRIO 0                   /*!< Trigger signal timer IRQ subpriority @hideinitializer */
#define REAR_TRIGGER_TIMER_PER_BUS RCC->APB1ENR            /*!< Trigger signal timer peripheral bus @hideinitializer */
#define REAR_TRIGGER_TIMER_PER_BUS_MASK RCC_APB1ENR_TIM3EN /*!< Trigger signal timer peripheral bus mask @hideinitializer */

#define GPIOA_STLINK_MODER_MASK 0xFC000000 /*!< Mask to clear the bits of the GPIOA pins used by the ST-LINK in the MODER register */
#define GPIOA_STLINK_PUPDR_MASK 0xFC000000 /*!< Mask to clear the bits of the GPIOA pins used by the ST-LINK in the PUPDR register */
/* Private variables ---------------------------------------------------------*/
static char msg[200]; /*!< Buffer for the error messages */

/* Private functions ----------------------------------------------------------*/
void setUp(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
}

void tearDown(void)
{
    RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOAEN;
    RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOBEN;    
}

void test_identifiers(void)
{
    UNITY_TEST_ASSERT_EQUAL_INT(0, PORT_REAR_PARKING_SENSOR_ID, __LINE__, "ERROR: PORT_REAR_PARKING_SENSOR_ID must be 0");
}

void test_trigger_pins(void)
{
    UNITY_TEST_ASSERT_EQUAL_INT(GPIOB, STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO, __LINE__, "ERROR: STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO GPIO must be GPIOB");
    UNITY_TEST_ASSERT_EQUAL_INT(0, STM32F4_REAR_PARKING_SENSOR_TRIGGER_PIN, __LINE__, "ERROR: STM32F4_REAR_PARKING_SENSOR_TRIGGER_PIN pin must be 0");
}

// Test trigger configuration
void _test_trigger_regs(void)
{
    // Retrieve previous configuration
    uint32_t prev_gpio_mode = STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO->MODER;
    uint32_t prev_gpio_pupd = STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO->PUPDR;

    // Call configuration function
    port_ultrasound_init(TEST_PORT_REAR_PARKING_SENSOR_ID);

    // Check that the mode is configured correctly
    uint32_t trigger_mode = ((STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO->MODER) >> (STM32F4_REAR_PARKING_SENSOR_TRIGGER_PIN * 2)) & GPIO_MODER_MODER0_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(STM32F4_GPIO_MODE_OUT, trigger_mode, __LINE__, "ERROR: Ultrasound trigger mode is not configured as output");

    // Check that the pull up/down is configured correctly
    uint32_t trigger_pupd = ((STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO->PUPDR) >> (STM32F4_REAR_PARKING_SENSOR_TRIGGER_PIN * 2)) & GPIO_PUPDR_PUPD0_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(STM32F4_GPIO_PUPDR_NOPULL, trigger_pupd, __LINE__, "ERROR: Ultrasound trigger pull up/down is not configured as no pull up/down");

    // Check that no other pins other than the needed have been modified:
    uint32_t mask = ~(GPIO_MODER_MODER0_Msk << (STM32F4_REAR_PARKING_SENSOR_TRIGGER_PIN * 2));
    uint32_t prev_gpio_mode_masked = prev_gpio_mode & mask;

    mask = ~(GPIO_PUPDR_PUPD0_Msk << (STM32F4_REAR_PARKING_SENSOR_TRIGGER_PIN * 2));
    uint32_t prev_gpio_pupd_masked = prev_gpio_pupd & mask;

    uint32_t curr_gpio_mode_masked = STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO->MODER & mask;
    uint32_t curr_gpio_pupd_masked = STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO->PUPDR & mask;

    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_gpio_mode_masked, curr_gpio_mode_masked, __LINE__, "ERROR: GPIO MODE has been modified for other pins than the trigger");
    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_gpio_pupd_masked, curr_gpio_pupd_masked, __LINE__, "ERROR: GPIO PUPD has been modified for other pins than the trigger");
}

void test_trigger_regs(void)
{
    GPIOB->MODER = ~0;
    GPIOB->PUPDR = ~0;
    _test_trigger_regs();
    GPIOB->MODER = 0;
    GPIOB->PUPDR = 0;
    _test_trigger_regs();
}

/**
 * @brief Test the configuration of the timer that controls the duration of the trigger signal of the ultrasound sensor.
 *
 */
void test_trigger_timer_config(void)
{
    // Retrieve previous configuration
    uint32_t prev_tim_trigger_cr1 = REAR_TRIGGER_TIMER->CR1;
    uint32_t prev_tim_trigger_dier = REAR_TRIGGER_TIMER->DIER;
    uint32_t prev_tim_trigger_sr = REAR_TRIGGER_TIMER->SR;

    // Call configuration function
    port_ultrasound_init(TEST_PORT_REAR_PARKING_SENSOR_ID);

    // Check that the ULTRASOUND timer for trigger signal is enabled in RCC
    uint32_t tim_trigger_rcc = (REAR_TRIGGER_TIMER_PER_BUS)&REAR_TRIGGER_TIMER_PER_BUS_MASK;
    UNITY_TEST_ASSERT_EQUAL_UINT32(REAR_TRIGGER_TIMER_PER_BUS_MASK, tim_trigger_rcc, __LINE__, "ERROR: ULTRASOUND timer for trigger signal is not enabled in RCC");

    // Check that the ULTRASOUND timer for trigger signal is disabled
    uint32_t tim_trigger_en = (REAR_TRIGGER_TIMER->CR1) & TIM_CR1_CEN_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(0, tim_trigger_en, __LINE__, "ERROR: ULTRASOUND timer for trigger signal must be disabled after configuration");

    // Check that the ULTRASOUND timer for trigger signal is configured with auto-reload preload enabled
    uint32_t tim_trigger_arpe = (REAR_TRIGGER_TIMER->CR1) & TIM_CR1_ARPE_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(TIM_CR1_ARPE_Msk, tim_trigger_arpe, __LINE__, "ERROR: ULTRASOUND timer for trigger signal must be configured with auto-reload preload enabled");

    // Check that the ULTRASOUND timer for trigger signal has cleared the update interrupt
    uint32_t tim_trigger_sr = (REAR_TRIGGER_TIMER->SR) & TIM_SR_UIF_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(0, tim_trigger_sr, __LINE__, "ERROR: ULTRASOUND timer for trigger signal must have cleared the update interrupt");

    // Check that the ULTRASOUND timer for trigger signal has cleared the interrupt
    uint32_t tim_trigger_dier = (REAR_TRIGGER_TIMER->DIER) & TIM_DIER_UIE_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(TIM_DIER_UIE_Msk, tim_trigger_dier, __LINE__, "ERROR: ULTRASOUND timer for trigger signal must have enabled the interrupt");

    // Check that no other bits other than the needed have been modified:
    uint32_t prev_tim_trigger_cr1_masked = prev_tim_trigger_cr1 & ~(TIM_CR1_ARPE_Msk | TIM_CR1_CEN_Msk);
    uint32_t prev_tim_trigger_dier_masked = prev_tim_trigger_dier & ~TIM_DIER_UIE_Msk;
    uint32_t prev_tim_trigger_sr_masked = prev_tim_trigger_sr & ~TIM_SR_UIF_Msk;

    uint32_t curr_tim_trigger_cr1_masked = REAR_TRIGGER_TIMER->CR1 & ~(TIM_CR1_ARPE_Msk | TIM_CR1_CEN_Msk);
    uint32_t curr_tim_trigger_dier_masked = REAR_TRIGGER_TIMER->DIER & ~TIM_DIER_UIE_Msk;
    uint32_t curr_tim_trigger_sr_masked = REAR_TRIGGER_TIMER->SR & ~TIM_SR_UIF_Msk;

    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_tim_trigger_cr1_masked, curr_tim_trigger_cr1_masked, __LINE__, "ERROR: The register CR1 of the ULTRASOUND timer for trigger signal has been modified for other bits than the needed");
    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_tim_trigger_dier_masked, curr_tim_trigger_dier_masked, __LINE__, "ERROR: The register DIER of the ULTRASOUND timer for trigger signal has been modified for other bits than the needed");
    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_tim_trigger_sr_masked, curr_tim_trigger_sr_masked, __LINE__, "ERROR: The register SR of the ULTRASOUND timer for trigger signal has been modified for other bits than the needed");
}

void test_trigger_timer_priority(void)
{
    uint32_t Priority = NVIC_GetPriority(REAR_TRIGGER_TIMER_IRQ);
    uint32_t PriorityGroup = NVIC_GetPriorityGrouping();
    uint32_t pPreemptPriority;
    uint32_t pSubPriority;

    NVIC_DecodePriority(Priority, PriorityGroup, &pPreemptPriority, &pSubPriority);

    TEST_ASSERT_EQUAL(REAR_TRIGGER_TIMER_IRQ_PRIO, pPreemptPriority);
    TEST_ASSERT_EQUAL(REAR_TRIGGER_TIMER_IRQ_SUBPRIO, pSubPriority);
}

/**
 * @brief Test the configuration of the ULTRASOUND trigger signal
 *
 */
void test_trigger_timer_duration()
{
    uint32_t prev_tim_trigger_cr1 = REAR_TRIGGER_TIMER->CR1;

    // Call configuration function to set the trigger signal
    port_ultrasound_init(TEST_PORT_REAR_PARKING_SENSOR_ID);

    // Disable ULTRASOUND trigger signal interrupts to avoid any interference
    NVIC_DisableIRQ(REAR_TRIGGER_TIMER_IRQ);

    // Check the computation of the ARR and PSC for the ULTRASOUND trigger signal
    uint32_t us_test = 10;
    uint32_t arr = REAR_TRIGGER_TIMER->ARR;
    uint32_t psc = REAR_TRIGGER_TIMER->PSC;
    uint32_t tim_trigger_dur_us = round((((double)(arr) + 1.0) / ((double)SystemCoreClock / 1000000.0)) * ((double)(psc) + 1));
    sprintf(msg, "ERROR: ULTRASOUND timer for trigger signal ARR and PSC are not configured correctly for a duration of %ld us", us_test);
    UNITY_TEST_ASSERT_INT_WITHIN(1, us_test, tim_trigger_dur_us, __LINE__, msg);

    // Check that the ULTRASOUND timer for trigger signal is enabled
    uint32_t tim_trigger_en = (REAR_TRIGGER_TIMER->CR1) & TIM_CR1_CEN_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(false, tim_trigger_en, __LINE__, "ERROR: ULTRASOUND timer for trigger should not be enabled  after setting the configuration");

    // Check that the trigger_end flag is cleared
    bool trigger_end = port_ultrasound_get_trigger_end(TEST_PORT_REAR_PARKING_SENSOR_ID);
    UNITY_TEST_ASSERT_EQUAL_UINT32(false, trigger_end, __LINE__, "ERROR: ULTRASOUND trigger_end flag must be cleared after setting the configuration");

    // Check that the meas_end flag is cleared
    bool trigger_ready = port_ultrasound_get_trigger_ready(TEST_PORT_REAR_PARKING_SENSOR_ID);
    UNITY_TEST_ASSERT_EQUAL_UINT32(true, trigger_ready, __LINE__, "ERROR: ULTRASOUND trigger_ready flag must be set after setting the configuration");

    // Check that no other bits other than the needed have been modified:
    uint32_t prev_tim_trigger_cr1_masked = prev_tim_trigger_cr1 & ~TIM_CR1_CEN_Msk;
    uint32_t curr_tim_trigger_cr1_masked = REAR_TRIGGER_TIMER->CR1 & ~TIM_CR1_CEN_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_tim_trigger_cr1_masked, curr_tim_trigger_cr1_masked, __LINE__, "ERROR: The register CR1 of the ULTRASOUND timer for trigger signal has been modified for other bits than the needed");
}

void test_trigger_timer_timeout(void)
{
    // Call configuration function to set the trigger signal
    port_ultrasound_init(TEST_PORT_REAR_PARKING_SENSOR_ID);

    // Enable ULTRASOUND trigger signal interrupts to test the timeout
    NVIC_EnableIRQ(REAR_TRIGGER_TIMER_IRQ);

    // Enable the timer
    REAR_TRIGGER_TIMER->CR1 |= TIM_CR1_CEN;

    // Wait for the timeout
    port_system_delay_ms(1); // Wait a time higher than the trigger signal duration

    // Disable ULTRASOUND trigger signal interrupts to avoid any interference
    NVIC_DisableIRQ(REAR_TRIGGER_TIMER_IRQ);

    // Check that the trigger_end flag is set
    bool trigger_end = port_ultrasound_get_trigger_end(TEST_PORT_REAR_PARKING_SENSOR_ID);
    UNITY_TEST_ASSERT_EQUAL_UINT32(true, trigger_end, __LINE__, "ERROR: ULTRASOUND trigger_end flag must be set after the timeout");
}

/**
 * @brief Test the generalization of the trigger port driver. Particularly, test that the port driver functions work with the ultrasounds_arr array and not with the specific GPIOx peripheral.
 *
 */
void test_trigger_port_generalization(void)
{
    // Change the GPIO and pin in the ultrasounds_arr array to a different one
    GPIO_TypeDef *p_expected_gpio_port = GPIOC;
    uint8_t expected_gpio_pin = 6;
    stm32f4_ultrasound_set_new_trigger_gpio(TEST_PORT_REAR_PARKING_SENSOR_ID, p_expected_gpio_port, expected_gpio_pin);

    // TEST init function
    // Enable RCC for the specific GPIO
    if (STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO == GPIOA)
    {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; /* GPIOA_CLK_ENABLE */
    }
    else if (STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO == GPIOB)
    {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; /* GPIOB_CLK_ENABLE */
    }
    else if (STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO == GPIOC)
    {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; /* GPIOC_CLK_ENABLE */
    }

    // Clean/set all configurations
    SYSCFG->EXTICR[STM32F4_REAR_PARKING_SENSOR_TRIGGER_PIN / 4] = 0;
    EXTI->RTSR = 0;
    EXTI->FTSR = 0;
    EXTI->EMR = 0;
    EXTI->IMR = 0;

    STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO->MODER |= (STM32F4_GPIO_MODE_IN << (STM32F4_REAR_PARKING_SENSOR_TRIGGER_PIN * 2U));      // Wrong configuration to check that the GPIO is not being modified
    STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO->PUPDR |= (STM32F4_GPIO_PUPDR_PULLUP << (STM32F4_REAR_PARKING_SENSOR_TRIGGER_PIN * 2U)); // Wrong configuration to check that the GPIO is not being modified

    // Disable RCC for the specific GPIO
    if (STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO == GPIOA)
    {
        RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOAEN;
    }
    else if (STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO == GPIOB)
    {
        RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOBEN;
    }
    else if (STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO == GPIOC)
    {
        RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOCEN;
    }

    // Expected configuration of the GPIO
    stm32f4_system_gpio_config(p_expected_gpio_port, expected_gpio_pin, STM32F4_GPIO_MODE_OUT, STM32F4_GPIO_PUPDR_NOPULL);
    uint32_t expected_gpio_mode = STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO->MODER;
    uint32_t expected_gpio_pupd = STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO->PUPDR;

    // Call configuration function
    port_ultrasound_init(TEST_PORT_REAR_PARKING_SENSOR_ID);

    // Check that the expected GPIO peripheral has not been modified. Otherwise, the port driver is not generalizing the GPIO peripheral and it is not working with the ultrasounds_arr array but with the specific GPIO nad pin.
    uint32_t curr_gpio_mode = STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO->MODER;
    uint32_t curr_gpio_pupd = STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO->PUPDR;

    UNITY_TEST_ASSERT_EQUAL_UINT32(expected_gpio_mode, curr_gpio_mode, __LINE__, "ERROR: The configuration function is not generalizing the GPIO and/or pin but working with the specific GPIO and pin for the trigger signal");
    UNITY_TEST_ASSERT_EQUAL_UINT32(expected_gpio_pupd, curr_gpio_pupd, __LINE__, "ERROR: The configuration function is not generalizing the GPIO and/or pin but working with the specific GPIO and pin for the trigger signal");
}

int main(void)
{
    port_system_init();
    UNITY_BEGIN();
    // Test ultrasound port driver
    RUN_TEST(test_identifiers);

    // Test ultrasound trigger signal configuration
    RUN_TEST(test_trigger_pins);
    RUN_TEST(test_trigger_regs);
    RUN_TEST(test_trigger_timer_config);
    RUN_TEST(test_trigger_timer_priority);
    RUN_TEST(test_trigger_timer_duration);
    RUN_TEST(test_trigger_timer_timeout);

    // Test generalization of the port driver
    RUN_TEST(test_trigger_port_generalization);
    
    exit(UNITY_END());
}
