/**
 * @file test_port_display.c
 * @brief Unit test for the display port driver.
 *
 * It checks the configuration of the display peripheral and the GPIO pins and other configurations using the Unity framework.
 *
 * @author Sistemas Digitales II
 * @date 2025-01-01
 */

/* Includes ------------------------------------------------------------------*/
/* HW independent libraries */
#include <stdlib.h>
#include <unity.h>

/* HW dependent libraries */
#include "port_display.h"
#include "port_system.h"
#include "stm32f4_system.h"
#include "stm32f4_display.h"
#include "stm32f4xx.h"

/* Defines and enums ----------------------------------------------------------*/
#define TEST_PORT_REAR_PARKING_DISPLAY_ID 0 /*!< Display identifier @hideinitializer */
#define TEST_PORT_DISPLAY_RGB_MAX_VALUE 255 /*!< Maximum value for the RGB color @hideinitializer */

// RGB pins configuration
#define TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_R_GPIO GPIOB     /*!< GPIO for the red color of the display @hideinitializer */
#define TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_R_PIN 6          /*!< Pin for the red color of the display sensor @hideinitializer */
#define TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_R_AF STM32F4_AF2 /*!< Red LED Alternate Function @hideinitializer */

#define TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_G_GPIO GPIOB     /*!< GPIO for the green color of the display @hideinitializer */
#define TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_G_PIN 8          /*!< Pin for the green color of the display sensor @hideinitializer */
#define TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_G_AF STM32F4_AF2 /*!< Green LED Alternate Function @hideinitializer */
#define TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_B_GPIO GPIOB     /*!< GPIO for the blue color of the display @hideinitializer */
#define TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_B_PIN 9          /*!< Pin for the blue color of the display sensor @hideinitializer */
#define TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_B_AF STM32F4_AF2 /*!< Blue LED Alternate Function @hideinitializer */

// RGB timer configuration
#define DISPLAY_RGB_PWM TIM4                            /*!< Display RGB timer @hideinitializer */
#define DISPLAY_RGB_PWM_PER_BUS RCC->APB1ENR            /*!< Display RGB timer peripheral bus @hideinitializer */
#define DISPLAY_RGB_PWM_PER_BUS_MASK RCC_APB1ENR_TIM4EN /*!< Display RGB timer peripheral bus mask @hideinitializer */
#define DISPLAY_RGB_PWM_PERIOD_MS 20                    /*!< Period of the RGB display timer @hideinitializer */

/* Private variables ---------------------------------------------------------*/
static char msg[200]; /*!< Buffer for the error messages */

/* Private functions ----------------------------------------------------------*/
void setUp(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
}

void tearDown(void)
{
    RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOBEN;
}

void test_identifiers(void)
{
    sprintf(msg, "ERROR: PORT_REAR_PARKING_DISPLAY_ID must be %d", TEST_PORT_REAR_PARKING_DISPLAY_ID);
    UNITY_TEST_ASSERT_EQUAL_INT(TEST_PORT_REAR_PARKING_DISPLAY_ID, PORT_REAR_PARKING_DISPLAY_ID, __LINE__, msg);

    sprintf(msg, "ERROR: PORT_DISPLAY_RGB_MAX_VALUE must be %d", TEST_PORT_DISPLAY_RGB_MAX_VALUE);
    UNITY_TEST_ASSERT_EQUAL_INT(TEST_PORT_DISPLAY_RGB_MAX_VALUE, PORT_DISPLAY_RGB_MAX_VALUE, __LINE__, msg);
}

void test_trigger_pins(void)
{
    // Check RGB pins configuration
    // Red
    UNITY_TEST_ASSERT_EQUAL_INT(TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_R_GPIO, STM32F4_REAR_PARKING_DISPLAY_RGB_R_GPIO, __LINE__, "ERROR: STM32F4_REAR_PARKING_DISPLAY_RGB_R_GPIO GPIO must be GPIOB");
    sprintf(msg, "ERROR: STM32F4_REAR_PARKING_DISPLAY_RGB_R_PIN pin must be %d", TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_R_PIN);
    UNITY_TEST_ASSERT_EQUAL_INT(TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_R_PIN, STM32F4_REAR_PARKING_DISPLAY_RGB_R_PIN, __LINE__, msg);

    // Green
    UNITY_TEST_ASSERT_EQUAL_INT(TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_G_GPIO, STM32F4_REAR_PARKING_DISPLAY_RGB_G_GPIO, __LINE__, "ERROR: STM32F4_REAR_PARKING_DISPLAY_RGB_G_GPIO GPIO must be GPIOB");
    sprintf(msg, "ERROR: STM32F4_REAR_PARKING_DISPLAY_RGB_G_PIN pin must be %d", TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_G_PIN);
    UNITY_TEST_ASSERT_EQUAL_INT(TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_G_PIN, STM32F4_REAR_PARKING_DISPLAY_RGB_G_PIN, __LINE__, msg);

    // Blue
    UNITY_TEST_ASSERT_EQUAL_INT(TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_B_GPIO, STM32F4_REAR_PARKING_DISPLAY_RGB_B_GPIO, __LINE__, "ERROR: STM32F4_REAR_PARKING_DISPLAY_RGB_B_GPIO GPIO must be GPIOB");
    sprintf(msg, "ERROR: STM32F4_REAR_PARKING_DISPLAY_RGB_B_PIN pin must be %d", TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_B_PIN);
    UNITY_TEST_ASSERT_EQUAL_INT(TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_B_PIN, STM32F4_REAR_PARKING_DISPLAY_RGB_B_PIN, __LINE__, msg);
}

// Test configuration
void _test_trigger_regs(void)
{
    // Retrieve previous configuration
    uint32_t prev_gpio_mode = TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_R_GPIO->MODER;
    uint32_t prev_gpio_pupd = TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_R_GPIO->PUPDR;

    // Call configuration function
    port_display_init(TEST_PORT_REAR_PARKING_DISPLAY_ID);

    // Check that the mode is configured correctly
    uint32_t mode_r = ((TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_R_GPIO->MODER) >> (TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_R_PIN * 2)) & GPIO_MODER_MODER0_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(STM32F4_GPIO_MODE_AF, mode_r, __LINE__, "ERROR: Display mode pin is not configured as alternate for red LED");

    uint32_t mode_g = ((TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_G_GPIO->MODER) >> (TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_G_PIN * 2)) & GPIO_MODER_MODER0_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(STM32F4_GPIO_MODE_AF, mode_g, __LINE__, "ERROR: Display mode pin is not configured as alternate for green LED");

    uint32_t mode_b = ((TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_B_GPIO->MODER) >> (TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_B_PIN * 2)) & GPIO_MODER_MODER0_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(STM32F4_GPIO_MODE_AF, mode_b, __LINE__, "ERROR: Display mode pin is not configured as alternate for blue LED");

    // Check that the pull up/down is configured correctly
    uint32_t pupd_r = ((TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_R_GPIO->PUPDR) >> (TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_R_PIN * 2)) & GPIO_PUPDR_PUPD0_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(STM32F4_GPIO_PUPDR_NOPULL, pupd_r, __LINE__, "ERROR: Display pull up/down is not configured as no pull up/down for red LED");

    uint32_t pupd_g = ((TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_G_GPIO->PUPDR) >> (TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_G_PIN * 2)) & GPIO_PUPDR_PUPD0_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(STM32F4_GPIO_PUPDR_NOPULL, pupd_g, __LINE__, "ERROR: Display pull up/down is not configured as no pull up/down for green LED");

    uint32_t pupd_b = ((TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_B_GPIO->PUPDR) >> (TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_B_PIN * 2)) & GPIO_PUPDR_PUPD0_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(STM32F4_GPIO_PUPDR_NOPULL, pupd_b, __LINE__, "ERROR: Display pull up/down is not configured as no pull up/down for blue LED");

    // Check alternate function
    uint32_t red_af = ((TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_R_GPIO->AFR[TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_R_PIN / 8]) >> ((TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_R_PIN % 8) * 4)) & 0xF;
    sprintf(msg, "ERROR: Display red LED alternate function is not configured correctly as AF%d", TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_R_AF);
    UNITY_TEST_ASSERT_EQUAL_UINT32(TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_R_AF, red_af, __LINE__, msg);

    uint32_t green_af = ((TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_G_GPIO->AFR[TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_G_PIN / 8]) >> ((TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_G_PIN % 8) * 4)) & 0xF;
    sprintf(msg, "ERROR: Display green LED alternate function is not configured correctly as AF%d", TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_G_AF);
    UNITY_TEST_ASSERT_EQUAL_UINT32(TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_G_AF, green_af, __LINE__, msg);

    uint32_t blue_af = ((TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_B_GPIO->AFR[TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_B_PIN / 8]) >> ((TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_B_PIN % 8) * 4)) & 0xF;
    sprintf(msg, "ERROR: Display blue LED alternate function is not configured correctly as AF%d", TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_B_AF);
    UNITY_TEST_ASSERT_EQUAL_UINT32(TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_B_AF, blue_af, __LINE__, msg);

    // Check that no other pins other than the needed have been modified:
    uint32_t mask = ~(
        (GPIO_MODER_MODER0_Msk << (TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_R_PIN * 2)) |
        (GPIO_MODER_MODER0_Msk << (TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_G_PIN * 2)) |
        (GPIO_MODER_MODER0_Msk << (TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_B_PIN * 2)));
    uint32_t prev_gpio_mode_masked = prev_gpio_mode & mask;

    mask = ~(
        (GPIO_PUPDR_PUPD0_Msk << (TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_R_PIN * 2)) |
        (GPIO_PUPDR_PUPD0_Msk << (TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_G_PIN * 2)) |
        (GPIO_PUPDR_PUPD0_Msk << (TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_B_PIN * 2)));

    uint32_t prev_gpio_pupd_masked = prev_gpio_pupd & mask;

    uint32_t curr_gpio_mode_masked = TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_R_GPIO->MODER & mask;
    uint32_t curr_gpio_pupd_masked = TEST_STM32F4_REAR_PARKING_DISPLAY_RGB_R_GPIO->PUPDR & mask;

    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_gpio_mode_masked, curr_gpio_mode_masked, __LINE__, "ERROR: GPIO MODE has been modified for other pins than the needed for the RGB LED");
    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_gpio_pupd_masked, curr_gpio_pupd_masked, __LINE__, "ERROR: GPIO PUPD has been modified for other pins than the needed for the RGB LED");
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
 * @brief Test the configuration of the timer for the period of the RGB display
 *
 */
void test_display_timer_pwm_config(void)
{
    // Retrieve previous configuration
    uint32_t prev_tim_pwm_cr1 = DISPLAY_RGB_PWM->CR1;
    uint32_t prev_tim_pwm_ccer = DISPLAY_RGB_PWM->CCER;
    uint32_t prev_tim_pwm_ccr1 = DISPLAY_RGB_PWM->CCR1;
    uint32_t prev_tim_pwm_ccmr1 = DISPLAY_RGB_PWM->CCMR1;
    uint32_t prev_tim_pwm_ccmr2 = DISPLAY_RGB_PWM->CCMR2;

    // Call configuration function
    port_display_init(TEST_PORT_REAR_PARKING_DISPLAY_ID);

    // Check that the DISPLAY timer for PWM is enabled in RCC
    uint32_t tim_pwm_rcc = DISPLAY_RGB_PWM_PER_BUS & DISPLAY_RGB_PWM_PER_BUS_MASK;
    UNITY_TEST_ASSERT_EQUAL_UINT32(DISPLAY_RGB_PWM_PER_BUS_MASK, tim_pwm_rcc, __LINE__, "ERROR: DISPLAY timer for PWM is not enabled in RCC");

    // Check that the DISPLAY timer for PWM is disabled
    uint32_t tim_pwm_en = (DISPLAY_RGB_PWM->CR1) & TIM_CR1_CEN_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(0, tim_pwm_en, __LINE__, "ERROR: DISPLAY timer for PWM must be disabled after configuration");

    // Check that the DISPLAY timer for PWM is configured with auto-reload preload enabled
    uint32_t tim_pwm_arpe = (DISPLAY_RGB_PWM->CR1) & TIM_CR1_ARPE_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(TIM_CR1_ARPE_Msk, tim_pwm_arpe, __LINE__, "ERROR: DISPLAY timer for PWM must be configured with auto-reload preload enabled");

    // Cannot check that the DISPLAY timer for PWM has set the update generation bit. It is cleared by HW after the update event is generated.

    // Check that the ARR, PSC and CNT are configured correctly
    uint32_t arr = DISPLAY_RGB_PWM->ARR;
    uint32_t psc = DISPLAY_RGB_PWM->PSC;
    uint32_t tim_dur_ms = round((((double)(arr) + 1.0) / ((double)SystemCoreClock / 1000.0)) * ((double)(psc) + 1));
    sprintf(msg, "ERROR: DISPLAY PWM period duration ARR and PSC are not configured correctly for a duration of %d ms", DISPLAY_RGB_PWM_PERIOD_MS);
    UNITY_TEST_ASSERT_INT_WITHIN(1, DISPLAY_RGB_PWM_PERIOD_MS, tim_dur_ms, __LINE__, msg);

    UNITY_TEST_ASSERT_EQUAL_UINT32(0, DISPLAY_RGB_PWM->CNT, __LINE__, "ERROR: DISPLAY timer for PWM CNT must be cleared");

    // Check that the DISPLAY timer for PWM output compare is disabled
    uint32_t tim_pwm_ccer = (DISPLAY_RGB_PWM->CCER) & TIM_CCER_CC1E_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(0, tim_pwm_ccer, __LINE__, "ERROR: DISPLAY timer for PWM output compare must be disabled");

    // Check that the DISPLAY timer for PWM has configured the PWM mode correctly
    uint32_t tim_pwm_ccmr1 = (DISPLAY_RGB_PWM->CCMR1) & (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1); // Channel 1
    UNITY_TEST_ASSERT_EQUAL_UINT32((TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1), tim_pwm_ccmr1, __LINE__, "ERROR: DISPLAY timer for PWM has not configured the PWM mode correctly for red LED.");

    uint32_t tim_pwm_ccmr2 = (DISPLAY_RGB_PWM->CCMR2) & ((TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1) | (TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1)); // Channel 3 and 4
    UNITY_TEST_ASSERT_EQUAL_UINT32((TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1) | (TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1), tim_pwm_ccmr2, __LINE__, "ERROR: DISPLAY timer for PWM has not configured the PWM mode correctly for green and blue LED.");

    // Check that the DISPLAY timer for PWM has configured the preload register correctly
    uint32_t tim_pwm_ccmr1_preload = (DISPLAY_RGB_PWM->CCMR1) & TIM_CCMR1_OC1PE_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(TIM_CCMR1_OC1PE_Msk, tim_pwm_ccmr1_preload, __LINE__, "ERROR: DISPLAY timer for PWM has not configured the preload register correctly for red LED.");

    uint32_t tim_pwm_ccmr2_preload = (DISPLAY_RGB_PWM->CCMR2) & (TIM_CCMR2_OC3PE_Msk | TIM_CCMR2_OC4PE_Msk);
    UNITY_TEST_ASSERT_EQUAL_UINT32(TIM_CCMR2_OC3PE_Msk | TIM_CCMR2_OC4PE_Msk, tim_pwm_ccmr2_preload, __LINE__, "ERROR: DISPLAY timer for PWM has not configured the preload register correctly for green and blue LED.");

    // Check that no other bits other than the needed have been modified:
    uint32_t prev_tim_pwm_cr1_masked = prev_tim_pwm_cr1 & ~(TIM_CR1_ARPE_Msk | TIM_CR1_CEN_Msk);
    uint32_t prev_tim_pwm_ccer_masked = prev_tim_pwm_ccer & ~TIM_CCER_CC1E_Msk;
    uint32_t prev_tim_pwm_ccr1_masked = prev_tim_pwm_ccr1 & 0xFFFFU;
    uint32_t prev_tim_pwm_ccr2_masked = prev_tim_pwm_ccmr2 & 0xFFFFU;
    uint32_t prev_tim_pwm_ccmr1_masked = prev_tim_pwm_ccmr1 & ~(TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE_Msk);
    uint32_t prev_tim_pwm_ccmr2_masked = prev_tim_pwm_ccmr2 & ~((TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1) | (TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1));

    uint32_t curr_tim_pwm_cr1_masked = DISPLAY_RGB_PWM->CR1 & ~(TIM_CR1_ARPE_Msk | TIM_CR1_CEN_Msk);
    uint32_t curr_tim_pwm_ccer_masked = DISPLAY_RGB_PWM->CCER & ~TIM_CCER_CC1E_Msk;
    uint32_t curr_tim_pwm_ccr1_masked = DISPLAY_RGB_PWM->CCR1 & 0xFFFFU;
    uint32_t curr_tim_pwm_ccr2_masked = DISPLAY_RGB_PWM->CCMR2 & 0xFFFFU;
    uint32_t curr_tim_pwm_ccmr1_masked = DISPLAY_RGB_PWM->CCMR1 & ~(TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE_Msk);
    uint32_t curr_tim_pwm_ccmr2_masked = DISPLAY_RGB_PWM->CCMR2 & ~((TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1) | (TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1));

    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_tim_pwm_cr1_masked, curr_tim_pwm_cr1_masked, __LINE__, "ERROR: The register CR1 of the DISPLAY timer for PWM has been modified for other bits than the needed");
    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_tim_pwm_ccer_masked, curr_tim_pwm_ccer_masked, __LINE__, "ERROR: The register CCER of the DISPLAY timer for PWM has been modified for other bits than the needed");
    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_tim_pwm_ccr1_masked, curr_tim_pwm_ccr1_masked, __LINE__, "ERROR: The register CCR1 of the DISPLAY timer for PWM has been modified and the duty cycle should not have been configured yet");
    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_tim_pwm_ccr2_masked, curr_tim_pwm_ccr2_masked, __LINE__, "ERROR: The register CCR2 of the DISPLAY timer for PWM has been modified and it should not have been changed");
    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_tim_pwm_ccmr1_masked, curr_tim_pwm_ccmr1_masked, __LINE__, "ERROR: The register CCMR1 of the DISPLAY timer for PWM has been modified for other bits than the needed");
    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_tim_pwm_ccmr2_masked, curr_tim_pwm_ccmr2_masked, __LINE__, "ERROR: The register CCMR2 of the DISPLAY timer for PWM has been modified and it should not have been changed");
}

void _test_display_set_color(rgb_color_t color)
{
    port_display_set_rgb(TEST_PORT_REAR_PARKING_DISPLAY_ID, color);

    uint32_t red_real = color.r;
    uint32_t green_real = color.g;
    uint32_t blue_real = color.b;

    // Check that the duty cycle is configured correctly
    uint32_t arr = DISPLAY_RGB_PWM->ARR;
    uint32_t ccr_red = DISPLAY_RGB_PWM->CCR1;
    uint32_t ccr_green = DISPLAY_RGB_PWM->CCR3;
    uint32_t ccr_blue = DISPLAY_RGB_PWM->CCR4;

    uint32_t red_test = ((ccr_red + 1) * TEST_PORT_DISPLAY_RGB_MAX_VALUE) / (arr + 1);
    uint32_t green_test = ((ccr_green + 1) * TEST_PORT_DISPLAY_RGB_MAX_VALUE) / (arr + 1);
    uint32_t blue_test = ((ccr_blue + 1) * TEST_PORT_DISPLAY_RGB_MAX_VALUE) / (arr + 1);

    sprintf(msg, "ERROR: DISPLAY red LED duty cycle is not configured correctly. Check CCRx and/or ARR  registers. Expected red level: %ld, actual: %ld", red_real, red_test);
    UNITY_TEST_ASSERT_UINT32_WITHIN(1, red_real, red_test, __LINE__, msg);

    sprintf(msg, "ERROR: DISPLAY green LED duty cycle is not configured correctly. Check CCRx and/or ARR  registers. Expected green level: %ld, actual: %ld", green_real, green_test);
    UNITY_TEST_ASSERT_UINT32_WITHIN(1, green_real, green_test, __LINE__, msg);

    sprintf(msg, "ERROR: DISPLAY blue LED duty cycle is not configured correctly. Check CCRx and/or ARR  registers. Expected blue level: %ld, actual: %ld", blue_real, blue_test);
    UNITY_TEST_ASSERT_UINT32_WITHIN(1, blue_real, blue_test, __LINE__, msg);
}

/**
 * @brief Test the configuration of the DISPLAY note frequency
 *
 */
void test_display_set_color(void)
{
    uint32_t prev_tim_pwm_cr1 = DISPLAY_RGB_PWM->CR1;
    uint32_t prev_tim_pwm_ccer = DISPLAY_RGB_PWM->CCER;
    uint32_t prev_tim_pwm_ccmr1 = DISPLAY_RGB_PWM->CCMR1;
    uint32_t prev_tim_pwm_ccmr2 = DISPLAY_RGB_PWM->CCMR2;

    // Check the computation of the ARR and PSC for the DISPLAY color
    // Minimum values
    rgb_color_t color_test_min = {0, 0, 0};
    _test_display_set_color(color_test_min);

    // Intermediate values
    rgb_color_t color_test_inter = {TEST_PORT_DISPLAY_RGB_MAX_VALUE / 2, TEST_PORT_DISPLAY_RGB_MAX_VALUE / 2, TEST_PORT_DISPLAY_RGB_MAX_VALUE / 2};
    _test_display_set_color(color_test_inter);

    // Maximum values
    rgb_color_t color_test_max = {TEST_PORT_DISPLAY_RGB_MAX_VALUE, TEST_PORT_DISPLAY_RGB_MAX_VALUE, TEST_PORT_DISPLAY_RGB_MAX_VALUE};
    _test_display_set_color(color_test_max);

    // Check that the DISPLAY timer for PWM is enabled
    uint32_t tim_pwm_en = (DISPLAY_RGB_PWM->CR1) & TIM_CR1_CEN_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(TIM_CR1_CEN_Msk, tim_pwm_en, __LINE__, "ERROR: DISPLAY timer for PWM must be enabled after setting the RGB color");

    // Check that the capture/compare enable bit is enabled for each channel
    uint32_t tim_pwm_ccer = (DISPLAY_RGB_PWM->CCER) & (TIM_CCER_CC1E_Msk | TIM_CCER_CC3E_Msk | TIM_CCER_CC4E_Msk);
    UNITY_TEST_ASSERT_EQUAL_UINT32(TIM_CCER_CC1E_Msk | TIM_CCER_CC3E_Msk | TIM_CCER_CC4E_Msk, tim_pwm_ccer, __LINE__, "ERROR: DISPLAY timer for PWM output compare must be enabled (CCER) for all channels after setting the RGB color");
    
    // Check that no other bits other than the needed have been modified:
    uint32_t prev_tim_pwm_cr1_masked = prev_tim_pwm_cr1 & ~TIM_CR1_CEN_Msk;
    uint32_t prev_tim_pwm_ccer_masked = prev_tim_pwm_ccer & ~TIM_CCER_CC1E_Msk;

    uint32_t curr_tim_pwm_cr1_masked = DISPLAY_RGB_PWM->CR1 & ~TIM_CR1_CEN_Msk;
    uint32_t curr_tim_pwm_ccer_masked = DISPLAY_RGB_PWM->CCER & ~(TIM_CCER_CC1E_Msk | TIM_CCER_CC3E_Msk | TIM_CCER_CC4E_Msk);
    uint32_t curr_tim_pwm_ccmr1 = DISPLAY_RGB_PWM->CCMR1;
    uint32_t curr_tim_pwm_ccmr2 = DISPLAY_RGB_PWM->CCMR2;

    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_tim_pwm_cr1_masked, curr_tim_pwm_cr1_masked, __LINE__, "ERROR: The register CR1 of the DISPLAY timer for PWM has been modified for other bits than the needed");
    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_tim_pwm_ccer_masked, curr_tim_pwm_ccer_masked, __LINE__, "ERROR: The register CCER of the DISPLAY timer for PWM has been modified for other bits than the needed");
    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_tim_pwm_ccmr1, curr_tim_pwm_ccmr1, __LINE__, "ERROR: The register CCMR1 of the DISPLAY timer for PWM has been modified and it should not have been changed");
    UNITY_TEST_ASSERT_EQUAL_UINT32(prev_tim_pwm_ccmr2, curr_tim_pwm_ccmr2, __LINE__, "ERROR: The register CCMR2 of the DISPLAY timer for PWM has been modified and it should not have been changed");
}

int main(void)
{
    port_system_init();
    UNITY_BEGIN();

    // Test display port driver
    RUN_TEST(test_identifiers);
    RUN_TEST(test_trigger_pins);
    RUN_TEST(test_trigger_regs);
    RUN_TEST(test_display_timer_pwm_config);
    RUN_TEST(test_display_set_color);

    exit(UNITY_END());
}
