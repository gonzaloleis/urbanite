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

// Echo timer configuration
#define REAR_ECHO_TIMER TIM2                             /*!< Echo signal timer @hideinitializer */
#define REAR_ECHO_TIMER_IRQ TIM2_IRQn                    /*!< Echo signal timer IRQ @hideinitializer */

// Measurement timer configuration
#define MEASUREMENT_TIMER TIM5                            /*!< Ultrasound measurement timer @hideinitializer */
#define MEASUREMENT_TIMER_PER_BUS RCC->APB1ENR            /*!< Ultrasound measurement timer peripheral bus @hideinitializer */
#define MEASUREMENT_TIMER_PER_BUS_MASK RCC_APB1ENR_TIM5EN /*!< Ultrasound measurement timer peripheral bus mask @hideinitializer */
#define MEASUREMENT_TIMER_IRQ TIM5_IRQn                   /*!< Ultrasound measurement timer IRQ @hideinitializer */
#define MEASUREMENT_TIMER_IRQ_PRIO 5                      /*!< Ultrasound measurement timer IRQ priority @hideinitializer */
#define MEASUREMENT_TIMER_IRQ_SUBPRIO 0                   /*!< Ultrasound measurement timer IRQ subpriority @hideinitializer */

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

/**
 * @brief Test the configuration of the timer that controls the measurement time of the ultrasound sensor.
 *
 */
void test_meas_timer_config(void)
{
    // Retrieve previous configuration
    uint32_t prev_tim_meas_cr1 = MEASUREMENT_TIMER->CR1;
    uint32_t prev_tim_meas_dier = MEASUREMENT_TIMER->DIER;
    uint32_t prev_tim_meas_sr = MEASUREMENT_TIMER->SR;

    // Call configuration function
    port_ultrasound_init(TEST_PORT_REAR_PARKING_SENSOR_ID);

    // Check that the ULTRASOUND timer for measurement is enabled in RCC
    uint32_t tim_meas_rcc = (MEASUREMENT_TIMER_PER_BUS)&MEASUREMENT_TIMER_PER_BUS_MASK;
    UNITY_TEST_ASSERT_EQUAL_UINT32(MEASUREMENT_TIMER_PER_BUS_MASK, tim_meas_rcc, __LINE__, "ERROR: ULTRASOUND timer for measurement is not enabled in RCC");

    // Check that the ULTRASOUND timer for measurement is disabled
    uint32_t tim_meas_en = (MEASUREMENT_TIMER->CR1) & TIM_CR1_CEN_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(0, tim_meas_en, __LINE__, "ERROR: ULTRASOUND timer for measurement must be disabled after configuration");

    // Check that the ULTRASOUND timer for measurement is configured with auto-reload preload enabled
    uint32_t tim_meas_arpe = (MEASUREMENT_TIMER->CR1) & TIM_CR1_ARPE_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(TIM_CR1_ARPE_Msk, tim_meas_arpe, __LINE__, "ERROR: ULTRASOUND timer for measurement must be configured with auto-reload preload enabled");

    // Check that the ULTRASOUND timer for measurement has cleared the update interrupt
    uint32_t tim_meas_sr = (MEASUREMENT_TIMER->SR) & TIM_SR_UIF_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(0, tim_meas_sr, __LINE__, "ERROR: ULTRASOUND timer for measurement must have cleared the update interrupt");

    // Check that the ULTRASOUND timer for measurement has cleared the interrupt
    uint32_t tim_meas_dier = (MEASUREMENT_TIMER->DIER) & TIM_DIER_UIE_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(TIM_DIER_UIE_Msk, tim_meas_dier, __LINE__, "ERROR: ULTRASOUND timer for measurement must have enabled the interrupt");

    // Check that no other bits other than the needed have been modified:
    uint32_t prev_tim_meas_cr1_masked = prev_tim_meas_cr1 & ~(TIM_CR1_ARPE_Msk | TIM_CR1_CEN_Msk);
    uint32_t prev_tim_meas_dier_masked = prev_tim_meas_dier & ~TIM_DIER_UIE_Msk;
    uint32_t prev_tim_meas_sr_masked = prev_tim_meas_sr & ~TIM_SR_UIF_Msk;

    uint32_t curr_tim_meas_cr1_masked = MEASUREMENT_TIMER->CR1 & ~(TIM_CR1_ARPE_Msk | TIM_CR1_CEN_Msk);
    uint32_t curr_tim_meas_dier_masked = MEASUREMENT_TIMER->DIER & ~TIM_DIER_UIE_Msk;
    uint32_t curr_tim_meas_sr_masked = MEASUREMENT_TIMER->SR & ~TIM_SR_UIF_Msk;

    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_tim_meas_cr1_masked, curr_tim_meas_cr1_masked, __LINE__, "ERROR: The register CR1 of the ULTRASOUND timer for measurement has been modified for other bits than the needed");
    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_tim_meas_dier_masked, curr_tim_meas_dier_masked, __LINE__, "ERROR: The register DIER of the ULTRASOUND timer for measurement has been modified for other bits than the needed");
    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_tim_meas_sr_masked, curr_tim_meas_sr_masked, __LINE__, "ERROR: The register SR of the ULTRASOUND timer for measurement has been modified for other bits than the needed");
}

/**
 * @brief Test the priority of the measurement timer.
 *
 */
void test_meas_timer_priority(void)
{
    uint32_t Priority = NVIC_GetPriority(MEASUREMENT_TIMER_IRQ);
    uint32_t PriorityGroup = NVIC_GetPriorityGrouping();
    uint32_t pPreemptPriority;
    uint32_t pSubPriority;

    NVIC_DecodePriority(Priority, PriorityGroup, &pPreemptPriority, &pSubPriority);

    TEST_ASSERT_EQUAL(MEASUREMENT_TIMER_IRQ_PRIO, pPreemptPriority);
    TEST_ASSERT_EQUAL(MEASUREMENT_TIMER_IRQ_SUBPRIO, pSubPriority);
}

/**
 * @brief Test the configuration of the ULTRASOUND measurement timer
 *
 */
void test_meas_timer_duration()
{
    uint32_t prev_tim_meas_cr1 = MEASUREMENT_TIMER->CR1;

    // Call configuration function to set the measurement
    port_ultrasound_init(TEST_PORT_REAR_PARKING_SENSOR_ID);

    // Disable ULTRASOUND measurement interrupts to avoid any interference
    NVIC_DisableIRQ(MEASUREMENT_TIMER_IRQ);

    // Check the computation of the ARR and PSC for the ULTRASOUND measurement
    uint32_t ms_test = 100;
    uint32_t arr = MEASUREMENT_TIMER->ARR;
    uint32_t psc = MEASUREMENT_TIMER->PSC;
    uint32_t tim_meas_dur_ms = round((((double)(arr) + 1.0) / ((double)SystemCoreClock / 1000.0)) * ((double)(psc) + 1));
    sprintf(msg, "ERROR: ULTRASOUND timer for measurement ARR and PSC are not configured correctly for a duration of %ld ms", ms_test);
    UNITY_TEST_ASSERT_INT_WITHIN(1, ms_test, tim_meas_dur_ms, __LINE__, msg);

    // Check that the ULTRASOUND timer for measurement is enabled
    uint32_t tim_meas_en = (MEASUREMENT_TIMER->CR1) & TIM_CR1_CEN_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(false, tim_meas_en, __LINE__, "ERROR: ULTRASOUND timer for measurements should not be enabled after setting the configuration");

    // Check that no other bits other than the needed have been modified:
    uint32_t prev_tim_meas_cr1_masked = prev_tim_meas_cr1 & ~TIM_CR1_CEN_Msk;
    uint32_t curr_tim_meas_cr1_masked = MEASUREMENT_TIMER->CR1 & ~TIM_CR1_CEN_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_tim_meas_cr1_masked, curr_tim_meas_cr1_masked, __LINE__, "ERROR: The register CR1 of the ULTRASOUND timer for measurement has been modified for other bits than the needed");
}

void test_meas_timer_timeout(void)
{
    // Call configuration function to set the measurement
    port_ultrasound_init(TEST_PORT_REAR_PARKING_SENSOR_ID);

    // Enable ULTRASOUND measurement interrupts to test the timeout
    NVIC_EnableIRQ(MEASUREMENT_TIMER_IRQ);

    // Enable the timer
    MEASUREMENT_TIMER->CR1 |= TIM_CR1_CEN;

    // Wait for the timeout
    port_system_delay_ms(101); // Wait a time higher than the measurement duration

    // Disable ULTRASOUND measurement interrupts to avoid any interference
    NVIC_DisableIRQ(MEASUREMENT_TIMER_IRQ);

    // Check that the meas_end flag is set
    bool trigger_ready = port_ultrasound_get_trigger_ready(TEST_PORT_REAR_PARKING_SENSOR_ID);
    UNITY_TEST_ASSERT_EQUAL_UINT32(true, trigger_ready, __LINE__, "ERROR: ULTRASOUND trigger_ready flag must be set after the measurement timer timeout");
}

void test_start_measurement(void)
{
    // Call configuration function to set the measurement
    port_ultrasound_init(TEST_PORT_REAR_PARKING_SENSOR_ID);

    // Call port function to start the measurement
    port_ultrasound_start_measurement(TEST_PORT_REAR_PARKING_SENSOR_ID);

    // Retrieve the current configuration of the NVIC interrupts before disabling them
    uint32_t tim_trigger_irq = NVIC->ISER[REAR_TRIGGER_TIMER_IRQ / 32] & (1 << (REAR_TRIGGER_TIMER_IRQ % 32));
    uint32_t tim_echo_irq = NVIC->ISER[REAR_ECHO_TIMER_IRQ / 32] & (1 << (REAR_ECHO_TIMER_IRQ % 32));
    uint32_t tim_meas_irq = NVIC->ISER[MEASUREMENT_TIMER_IRQ / 32] & (1 << (MEASUREMENT_TIMER_IRQ % 32));

    // Disable all interrupts to avoid any interference
    NVIC_DisableIRQ(REAR_TRIGGER_TIMER_IRQ);
    NVIC_DisableIRQ(REAR_ECHO_TIMER_IRQ);
    NVIC_DisableIRQ(MEASUREMENT_TIMER_IRQ);

    // Check that the trigger pin has been set to high
    uint32_t trigger_pin = STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO->ODR & (1 << STM32F4_REAR_PARKING_SENSOR_TRIGGER_PIN);
    
    UNITY_TEST_ASSERT_EQUAL_UINT32(1 << STM32F4_REAR_PARKING_SENSOR_TRIGGER_PIN, trigger_pin, __LINE__, "ERROR: The trigger pin must be set to high after starting the measurement");

    // Check that NVIC interrupts have been enabled for all the timers
    UNITY_TEST_ASSERT_EQUAL_UINT32(1 << (REAR_TRIGGER_TIMER_IRQ % 32), tim_trigger_irq, __LINE__, "ERROR: The NVIC interrupt for the ULTRASOUND trigger timer has not been enabled");

    UNITY_TEST_ASSERT_EQUAL_UINT32(1 << (REAR_ECHO_TIMER_IRQ % 32), tim_echo_irq, __LINE__, "ERROR: The NVIC interrupt for the ULTRASOUND echo timer has not been enabled");

    UNITY_TEST_ASSERT_EQUAL_UINT32(1 << (MEASUREMENT_TIMER_IRQ % 32), tim_meas_irq, __LINE__, "ERROR: The NVIC interrupt for the ULTRASOUND measurement timer has not been enabled");

    // Check that all the timers have been enabled
    uint32_t tim_trigger_en = (REAR_TRIGGER_TIMER->CR1) & TIM_CR1_CEN_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(TIM_CR1_CEN_Msk, tim_trigger_en, __LINE__, "ERROR: The ULTRASOUND trigger timer has not been enabled");

    uint32_t tim_echo_en = (REAR_ECHO_TIMER->CR1) & TIM_CR1_CEN_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(TIM_CR1_CEN_Msk, tim_echo_en, __LINE__, "ERROR: The ULTRASOUND echo timer has not been enabled");

    uint32_t tim_meas_en = (MEASUREMENT_TIMER->CR1) & TIM_CR1_CEN_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(TIM_CR1_CEN_Msk, tim_meas_en, __LINE__, "ERROR: The ULTRASOUND measurement timer has not been enabled");
}

int main(void)
{
    port_system_init();
    UNITY_BEGIN();
    // Test ultrasound port driver
    RUN_TEST(test_identifiers);

    // Test measurement timer configuration
    RUN_TEST(test_meas_timer_config);
    RUN_TEST(test_meas_timer_priority);
    RUN_TEST(test_meas_timer_duration);
    RUN_TEST(test_meas_timer_timeout);

    // Test start measurement
    RUN_TEST(test_start_measurement);

    exit(UNITY_END());
}
