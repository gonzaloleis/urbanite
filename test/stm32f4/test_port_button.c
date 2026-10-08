/**
 * @file test_port_button.c
 * @brief Unit test for the button port driver.
 *
 * It checks the configuration of the USER BUTTON peripheral and the GPIO pins. It also checks the priority of the button interrupt using the Unity framework.
 *
 * @author Sistemas Digitales II
 * @date 2025-01-01
 */

/* Includes ------------------------------------------------------------------*/
/* HW independent libraries */
#include <stdlib.h>
#include <unity.h>
#include "port_button.h"
#include "port_system.h"
/* HW dependent libraries */
#include "stm32f4_system.h"
#include "stm32f4_button.h"
#include "stm32f4xx.h"

/* Defines and enums ----------------------------------------------------------*/
/* Defines */
#define TEST_PORT_PARKING_BUTTON_ID 0   /*!< Button identifier @hideinitializer */
#define LD2_PORT GPIOA
#define LD2_PIN 5
#define LD2_DELAY_MS 100

void setUp(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    stm32f4_system_gpio_exti_disable(STM32F4_PARKING_BUTTON_PIN); // Disable interrupt of the wrong configuration to avoid interrupt calls    
}

void tearDown(void)
{
    RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOCEN;
}

void test_identifiers(void)
{
    UNITY_TEST_ASSERT_EQUAL_INT(0, PORT_PARKING_BUTTON_ID, __LINE__, "ERROR: PORT_REAR_PARKING_DISPLAY_ID must be 0");
}

void test_pins(void)
{
    UNITY_TEST_ASSERT_EQUAL_INT(GPIOC, STM32F4_PARKING_BUTTON_GPIO, __LINE__, "ERROR: USER_BUTTON GPIO must be GPIOC");
    UNITY_TEST_ASSERT_EQUAL_INT(13, STM32F4_PARKING_BUTTON_PIN, __LINE__, "ERROR: USER_BUTTON pin must be 13");
}

void _test_regs(void)
{
    // Retrieve previous configuration
    uint32_t prev_gpio_mode = STM32F4_PARKING_BUTTON_GPIO->MODER;
    uint32_t prev_gpio_pupd = STM32F4_PARKING_BUTTON_GPIO->PUPDR;

    // Call configuration function
    port_button_init(TEST_PORT_PARKING_BUTTON_ID);

    // Check that the mode is configured correctly
    uint32_t button_mode = ((STM32F4_PARKING_BUTTON_GPIO->MODER) >> (STM32F4_PARKING_BUTTON_PIN * 2)) & GPIO_MODER_MODER0_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(STM32F4_GPIO_MODE_IN, button_mode, __LINE__, "ERROR: Button mode is not configured as input");

    // Check that the pull up/down is configured correctly
    uint32_t button_pupd = ((STM32F4_PARKING_BUTTON_GPIO->PUPDR) >> (STM32F4_PARKING_BUTTON_PIN * 2)) & GPIO_PUPDR_PUPD0_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(STM32F4_GPIO_PUPDR_NOPULL, button_pupd, __LINE__, "ERROR: Button pull up/down is not configured as no pull up/down");

    // Check that no other pins other than the needed have been modified:
    uint32_t mask = ~(GPIO_MODER_MODER0_Msk << (STM32F4_PARKING_BUTTON_PIN * 2));
    uint32_t prev_gpio_mode_masked = prev_gpio_mode & mask;

    mask = ~(GPIO_PUPDR_PUPD0_Msk << (STM32F4_PARKING_BUTTON_PIN * 2));
    uint32_t prev_gpio_pupd_masked = prev_gpio_pupd & mask;

    uint32_t curr_gpio_mode_masked = STM32F4_PARKING_BUTTON_GPIO->MODER & mask;
    uint32_t curr_gpio_pupd_masked = STM32F4_PARKING_BUTTON_GPIO->PUPDR & mask;

    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_gpio_mode_masked, curr_gpio_mode_masked, __LINE__, "ERROR: GPIO MODE has been modified for other pins than the button");
    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_gpio_pupd_masked, curr_gpio_pupd_masked, __LINE__, "ERROR: GPIO PUPD has been modified for other pins than the button");
}

void test_regs(void)
{
    GPIOC->MODER = ~0;
    GPIOC->PUPDR = ~0;
    _test_regs();
    GPIOC->MODER = 0;
    GPIOC->PUPDR = 0;
    _test_regs();
}

void _test_exti(void)
{
    // Retrieve previous configuration
    uint32_t prev_gpio_exticr = SYSCFG->EXTICR[STM32F4_PARKING_BUTTON_PIN / 4];
    uint32_t prev_gpio_rtsr = EXTI->RTSR;
    uint32_t prev_gpio_ftsr = EXTI->FTSR;
    uint32_t prev_gpio_emr = EXTI->EMR;
    uint32_t prev_gpio_imr = EXTI->IMR;

    // Call configuration function
    port_button_init(TEST_PORT_PARKING_BUTTON_ID);

    // Check that the EXTI CR is configured correctly (EXTI both edges)
    uint32_t button_exticr = ((SYSCFG->EXTICR[STM32F4_PARKING_BUTTON_PIN / 4]) >> ((STM32F4_PARKING_BUTTON_PIN % 4) * 4)) & 0xF;
    UNITY_TEST_ASSERT_EQUAL_UINT32(0x2, button_exticr, __LINE__, "ERROR: Button EXTI CR is not configured correctly");

    // Check that the EXTI RTSR is configured correctly (Rising edge)
    uint32_t button_rtsr = ((EXTI->RTSR) >> STM32F4_PARKING_BUTTON_PIN) & 0x1;
    UNITY_TEST_ASSERT_EQUAL_UINT32(0x1, button_rtsr, __LINE__, "ERROR: Button EXTI RTSR is not configured correctly. It must be both rising and falling edge.");

    // Check that the EXTI FTSR is configured correctly (Falling edge)
    uint32_t button_ftsr = ((EXTI->FTSR) >> STM32F4_PARKING_BUTTON_PIN) & 0x1;
    UNITY_TEST_ASSERT_EQUAL_UINT32(0x1, button_ftsr, __LINE__, "ERROR: Button EXTI FTSR is not configured correctly. It must be both rising and falling edge.");

    // Check that the EXTI EMR is configured correctly (Event)
    uint32_t button_emr = ((EXTI->EMR) >> STM32F4_PARKING_BUTTON_PIN) & 0x1;
    UNITY_TEST_ASSERT_EQUAL_UINT32(0, button_emr, __LINE__, "ERROR: Button EXTI EMR is not configured correctly. It should not be in event mode.");

    // Check that the EXTI IMR is configured correctly (Interrupt)
    uint32_t button_imr = ((EXTI->IMR) >> STM32F4_PARKING_BUTTON_PIN) & 0x1;
    UNITY_TEST_ASSERT_EQUAL_UINT32(1, button_imr, __LINE__, "ERROR: Button EXTI IMR is not configured correctly. It must be in interrupt mode.");

    // Check that no other pins other than the needed have been modified:
    uint32_t mask_exticr = ~(0xF << ((STM32F4_PARKING_BUTTON_PIN % 4) * 4));
    uint32_t prev_gpio_exticr_masked = prev_gpio_exticr & mask_exticr;
    uint32_t curr_gpio_exticr_masked = SYSCFG->EXTICR[STM32F4_PARKING_BUTTON_PIN / 4] & mask_exticr;
    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_gpio_exticr_masked, curr_gpio_exticr_masked, __LINE__, "ERROR: EXTI CR has been modified for other ports than the button");

    uint32_t prev_gpio_rtsr_masked = prev_gpio_rtsr & ~(EXTI_RTSR_TR0_Msk << STM32F4_PARKING_BUTTON_PIN);
    uint32_t prev_gpio_ftsr_masked = prev_gpio_ftsr & ~(EXTI_FTSR_TR0_Msk << STM32F4_PARKING_BUTTON_PIN);
    uint32_t curr_gpio_rtsr_masked = EXTI->RTSR & ~(EXTI_RTSR_TR0_Msk << STM32F4_PARKING_BUTTON_PIN);
    uint32_t curr_gpio_ftsr_masked = EXTI->FTSR & ~(EXTI_FTSR_TR0_Msk << STM32F4_PARKING_BUTTON_PIN);
    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_gpio_rtsr_masked, curr_gpio_rtsr_masked, __LINE__, "ERROR: EXTI RTSR has been modified for other ports than the button");
    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_gpio_ftsr_masked, curr_gpio_ftsr_masked, __LINE__, "ERROR: EXTI FTSR has been modified for other ports than the button");

    uint32_t prev_gpio_emr_masked = prev_gpio_emr & ~(EXTI_EMR_MR0_Msk << STM32F4_PARKING_BUTTON_PIN);
    uint32_t prev_gpio_imr_masked = prev_gpio_imr & ~(EXTI_IMR_MR0_Msk << STM32F4_PARKING_BUTTON_PIN);
    uint32_t curr_gpio_emr_masked = EXTI->EMR & ~(EXTI_EMR_MR0_Msk << STM32F4_PARKING_BUTTON_PIN);
    uint32_t curr_gpio_imr_masked = EXTI->IMR & ~(EXTI_IMR_MR0_Msk << STM32F4_PARKING_BUTTON_PIN);
    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_gpio_emr_masked, curr_gpio_emr_masked, __LINE__, "ERROR: EXTI EMR has been modified for other ports than the button");
    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_gpio_imr_masked, curr_gpio_imr_masked, __LINE__, "ERROR: EXTI IMR has been modified for other ports than the button");
}

void test_exti(void)
{
    SYSCFG->EXTICR[STM32F4_PARKING_BUTTON_PIN / 4] = ~0;
    EXTI->RTSR = ~0;
    EXTI->FTSR = ~0;
    EXTI->EMR = ~0;
    EXTI->IMR = ~0;
    _test_exti();
    EXTI->RTSR = 0;
    EXTI->FTSR = 0;
    EXTI->EMR = 0;
    EXTI->IMR = 0;
    SYSCFG->EXTICR[STM32F4_PARKING_BUTTON_PIN / 4] = 0;
    _test_exti();
}

void test_write_gpio(void)
{
    // Configure the LD2 pin as output
    stm32f4_system_gpio_config(LD2_PORT, LD2_PIN, STM32F4_GPIO_MODE_OUT, STM32F4_GPIO_PUPDR_NOPULL);

    // Turn on and turn off the LD2 LED
    stm32f4_system_gpio_write(LD2_PORT, LD2_PIN, 1);
    port_system_delay_ms(LD2_DELAY_MS);
    UNITY_TEST_ASSERT_EQUAL_INT(1, (LD2_PORT->ODR >> LD2_PIN) & GPIO_ODR_OD0_Msk, __LINE__, "ERROR: LD2 LED is not turned on. The function stm32f4_system_gpio_write is not working properly");

    stm32f4_system_gpio_write(LD2_PORT, LD2_PIN, 0);
    port_system_delay_ms(LD2_DELAY_MS);
    UNITY_TEST_ASSERT_EQUAL_INT(0, (LD2_PORT->ODR >> LD2_PIN) & GPIO_ODR_OD0_Msk, __LINE__, "ERROR: LD2 LED is not turned off. The function stm32f4_system_gpio_write is not working properly");

    // Turn on and off the LD2 LED with the function stm32f4_system_gpio_toggle    
    LD2_PORT->ODR &= ~(1 << LD2_PIN); // Ensure that the LD2 LED is turned off
    stm32f4_system_gpio_toggle(LD2_PORT, LD2_PIN);
    port_system_delay_ms(LD2_DELAY_MS);
    UNITY_TEST_ASSERT_EQUAL_INT(1, (LD2_PORT->ODR >> LD2_PIN) & GPIO_ODR_OD0_Msk, __LINE__, "ERROR: LD2 LED is not turned on. The function stm32f4_system_gpio_toggle is not working properly");

    LD2_PORT->ODR |= (1 << LD2_PIN); // Ensure that the LD2 LED is turned on
    stm32f4_system_gpio_toggle(LD2_PORT, LD2_PIN);
    UNITY_TEST_ASSERT_EQUAL_INT(0, (LD2_PORT->ODR >> LD2_PIN) & GPIO_ODR_OD0_Msk, __LINE__, "ERROR: LD2 LED is not turned off. The function stm32f4_system_gpio_toggle is not working properly");
}

void test_exti_priority(void)
{
    uint32_t Priority = NVIC_GetPriority(EXTI15_10_IRQn);
    uint32_t PriorityGroup = NVIC_GetPriorityGrouping();
    uint32_t pPreemptPriority;
    uint32_t pSubPriority;

    NVIC_DecodePriority(Priority, PriorityGroup, &pPreemptPriority, &pSubPriority);

    TEST_ASSERT_EQUAL(1, pPreemptPriority);
    TEST_ASSERT_EQUAL(0, pSubPriority);
}

/**
 * @brief Test the generalization of the button port driver. Particularly, test that the port driver functions work with the buttons_arr array and not with the specific GPIOx peripheral.
 *
 */
void test_button_port_generalization(void)
{
    // Change the GPIO and pin in the buttons_arr array to a different one
    GPIO_TypeDef *p_expected_gpio_port = GPIOB;
    uint8_t expected_gpio_pin = 6;
    stm32f4_button_set_new_gpio(TEST_PORT_PARKING_BUTTON_ID, p_expected_gpio_port, expected_gpio_pin);

    // TEST init function
    // Enable RCC for the specific GPIO
    if (STM32F4_PARKING_BUTTON_GPIO == GPIOA)
    {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; /* GPIOA_CLK_ENABLE */
    }
    else if (STM32F4_PARKING_BUTTON_GPIO == GPIOB)
    {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; /* GPIOB_CLK_ENABLE */
    }
    else if (STM32F4_PARKING_BUTTON_GPIO == GPIOC)
    {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; /* GPIOC_CLK_ENABLE */
    }

    // Clean/set all configurations
    SYSCFG->EXTICR[STM32F4_PARKING_BUTTON_PIN / 4] = 0;    
    EXTI->RTSR = 0;
    EXTI->FTSR = 0;
    EXTI->EMR = 0;
    EXTI->IMR = 0;

    STM32F4_PARKING_BUTTON_GPIO->MODER |= (STM32F4_GPIO_MODE_OUT << (STM32F4_PARKING_BUTTON_PIN * 2U));  // Wrong configuration to check that the GPIO is not being modified
    STM32F4_PARKING_BUTTON_GPIO->PUPDR |= (STM32F4_GPIO_PUPDR_PULLUP << (STM32F4_PARKING_BUTTON_PIN * 2U)); // Wrong configuration to check that the GPIO is not being modified

    // Disable RCC for the specific GPIO
    if (STM32F4_PARKING_BUTTON_GPIO == GPIOA)
    {
        RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOAEN;
    }
    else if (STM32F4_PARKING_BUTTON_GPIO == GPIOB)
    {
        RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOBEN;
    }
    else if (STM32F4_PARKING_BUTTON_GPIO == GPIOC)
    {
        RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOCEN;
    }

    // Expected configuration of the GPIO    
    stm32f4_system_gpio_config(p_expected_gpio_port, expected_gpio_pin, STM32F4_GPIO_MODE_IN, STM32F4_GPIO_PUPDR_NOPULL);
    stm32f4_system_gpio_config_exti(p_expected_gpio_port, expected_gpio_pin, STM32F4_TRIGGER_BOTH_EDGE | STM32F4_TRIGGER_ENABLE_INTERR_REQ);
    stm32f4_system_gpio_exti_enable(expected_gpio_pin, 1, 0);
    uint32_t expected_gpio_mode = STM32F4_PARKING_BUTTON_GPIO->MODER;
    uint32_t expected_gpio_pupd = STM32F4_PARKING_BUTTON_GPIO->PUPDR;
    uint32_t expected_exticr = SYSCFG->EXTICR[STM32F4_PARKING_BUTTON_PIN / 4];
    uint32_t expected_rtsr = EXTI->RTSR;
    uint32_t expected_ftsr = EXTI->FTSR;
    uint32_t expected_emr = EXTI->EMR;
    uint32_t expected_imr = EXTI->IMR;

    // Get NVIC priority of the EXTI line for the specific pin
    NVIC_SetPriority(EXTI15_10_IRQn, 0);
    uint32_t expected_priority = NVIC_GetPriority(EXTI15_10_IRQn);

    // Call configuration function
    port_button_init(TEST_PORT_PARKING_BUTTON_ID);

    // Check that the expected GPIO peripheral has not been modified. Otherwise, the port driver is not generalizing the GPIO peripheral and it is not working with the buttons_arr array but with the specific GPIO nad pin.
    uint32_t curr_gpio_mode = STM32F4_PARKING_BUTTON_GPIO->MODER;
    uint32_t curr_gpio_pupd = STM32F4_PARKING_BUTTON_GPIO->PUPDR;
    uint32_t curr_exticr = SYSCFG->EXTICR[STM32F4_PARKING_BUTTON_PIN / 4];
    uint32_t curr_rtsr = EXTI->RTSR;
    uint32_t curr_ftsr = EXTI->FTSR;
    uint32_t curr_emr = EXTI->EMR;
    uint32_t curr_imr = EXTI->IMR;

    // Check that the NVIC priority of the EXTI line has not been modified. Otherwise, the port driver is not generalizing the EXTI line and it is not working with the buttons_arr array but with the specific GPIO and pin.
    uint32_t curr_priority = NVIC_GetPriority(EXTI15_10_IRQn);

    UNITY_TEST_ASSERT_EQUAL_UINT32(expected_gpio_mode, curr_gpio_mode, __LINE__, "ERROR: The configuration function is not generalizing the GPIO and/or pin but working with the specific GPIO and pin");
    UNITY_TEST_ASSERT_EQUAL_UINT32(expected_gpio_pupd, curr_gpio_pupd, __LINE__, "ERROR: The configuration function is not generalizing the GPIO and/or pin but working with the specific GPIO and pin");
    UNITY_TEST_ASSERT_EQUAL_UINT32(expected_exticr, curr_exticr, __LINE__, "ERROR: The configuration function is not generalizing the GPIO and/or pin but working with the specific GPIO and pin");
    UNITY_TEST_ASSERT_EQUAL_UINT32(expected_rtsr, curr_rtsr, __LINE__, "ERROR: The configuration function is not generalizing the GPIO and/or pin but working with the specific GPIO and pin");
    UNITY_TEST_ASSERT_EQUAL_UINT32(expected_ftsr, curr_ftsr, __LINE__, "ERROR: The configuration function is not generalizing the GPIO and/or pin but working with the specific GPIO and pin");
    UNITY_TEST_ASSERT_EQUAL_UINT32(expected_emr, curr_emr, __LINE__, "ERROR: The configuration function is not generalizing the GPIO and/or pin but working with the specific GPIO and pin");
    UNITY_TEST_ASSERT_EQUAL_UINT32(expected_imr, curr_imr, __LINE__, "ERROR: The configuration function is not generalizing the GPIO and/or pin but working with the specific GPIO and pin");
    UNITY_TEST_ASSERT_EQUAL_UINT32(expected_priority, curr_priority, __LINE__, "ERROR: The configuration function is not generalizing the GPIO and/or pin but working with the specific GPIO and pin");
}

int main(void)
{
    port_system_init();
    UNITY_BEGIN();
    RUN_TEST(test_identifiers);
    RUN_TEST(test_pins);
    RUN_TEST(test_regs);
    RUN_TEST(test_write_gpio);
    RUN_TEST(test_exti);
    RUN_TEST(test_exti_priority);
    // RUN_TEST(test_button_port_generalization); // Commented because it is not working properly in Windows. If you are using Linux or MacOS, uncomment this line.
    exit(UNITY_END());
}
