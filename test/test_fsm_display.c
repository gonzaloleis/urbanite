/**
 * @file test_fsm_display.c
 * @brief Unit test for the display FSM.
 *
 * @author Sistemas Digitales II
 * @date 2025-01-01
 */
/* System dependent libraries */
#include <stdlib.h>
#include <unity.h>

/* HW independent libraries */
#include "port_display.h"
#include "port_system.h"
#include "stm32f4_system.h"
#include "stm32f4_display.h"

/* Include FSM libraries */
#include "fsm.h"
#include "fsm_display.h"

/* Defines */
#define TEST_PORT_DISPLAY_RGB_MAX_VALUE 255 /*!< Maximum value for the RGB color @hideinitializer */

// RGB timer configuration
#define DISPLAY_RGB_PWM TIM4 /*!< Display RGB timer @hideinitializer */

/* Private variables ---------------------------------------------------------*/
static char msg[200]; /*!< Buffer for the error messages */
static fsm_display_t *p_fsm_display;

/* Private functions ----------------------------------------------------------*/
void setUp(void)
{
    p_fsm_display = fsm_display_new(PORT_REAR_PARKING_DISPLAY_ID);
}

void tearDown(void)
{
    // Nothing to do
}

void test_initial_config(void)
{
    fsm_t *p_inner_fsm = fsm_display_get_inner_fsm(p_fsm_display);
    UNITY_TEST_ASSERT_EQUAL_PTR(p_fsm_display, p_inner_fsm, __LINE__, "The inner FSM of fsm_display_t is not the first field of the struct");

    UNITY_TEST_ASSERT_EQUAL_INT(WAIT_DISPLAY, fsm_get_state(p_inner_fsm), __LINE__, "The initial state of the FSM is not WAIT_DISPLAY");

    // It assumes there are 3 transitions in the table plus the null transition
    fsm_trans_t *last_transition = &p_inner_fsm->p_tt[3];

    UNITY_TEST_ASSERT_EQUAL_INT(-1, last_transition->orig_state, __LINE__, "The origin state of the last transition of the FSM should be -1");
    UNITY_TEST_ASSERT_EQUAL_INT(NULL, last_transition->in, __LINE__, "The input condition function of the last transition of the FSM should be NULL");
    UNITY_TEST_ASSERT_EQUAL_INT(-1, last_transition->dest_state, __LINE__, "The destination state of the last transition of the FSM should be -1");
    UNITY_TEST_ASSERT_EQUAL_INT(NULL, last_transition->out, __LINE__, "The output modification function of the last transition of the FSM should be NULL");
}

/**
 * @brief Check the transition from WAIT_DISPLAY to SET_DISPLAY
 *
 */
void test_activation(void)
{
    // Check that the transition does not happen if the display is not active
    fsm_display_set_status(p_fsm_display, false);

    // Fire the transition
    fsm_display_fire(p_fsm_display);

    // Check the state
    fsm_t *p_inner_fsm = fsm_display_get_inner_fsm(p_fsm_display);
    UNITY_TEST_ASSERT_EQUAL_INT(WAIT_DISPLAY, fsm_get_state(p_inner_fsm), __LINE__, "The FSM should remain in the WAIT_DISPLAY state if the state of the display is not active");

    // Check that the transition happens if the display is active
    fsm_display_set_status(p_fsm_display, true);

    // Fire the transition
    fsm_display_fire(p_fsm_display);

    // Check the state
    UNITY_TEST_ASSERT_EQUAL_INT(SET_DISPLAY, fsm_get_state(p_inner_fsm), __LINE__, "The FSM should move to the SET_DISPLAY state if the state of the display is active");

    // Check that the output function is called and thus the color set is OFF
    uint32_t arr = DISPLAY_RGB_PWM->ARR;
    uint32_t ccr_red = DISPLAY_RGB_PWM->CCR1;
    uint32_t ccr_green = DISPLAY_RGB_PWM->CCR3;
    uint32_t ccr_blue = DISPLAY_RGB_PWM->CCR4;

    uint32_t red_test = (ccr_red * TEST_PORT_DISPLAY_RGB_MAX_VALUE) / (arr + 1);
    uint32_t green_test = (ccr_green * TEST_PORT_DISPLAY_RGB_MAX_VALUE) / (arr + 1);
    uint32_t blue_test = (ccr_blue * TEST_PORT_DISPLAY_RGB_MAX_VALUE) / (arr + 1);

    sprintf(msg, "ERROR: DISPLAY red LED is not OFF when the display is activated for the first time. Expected red level: %d, actual: %ld", 0, red_test);
    UNITY_TEST_ASSERT_UINT32_WITHIN(1, 0, red_test, __LINE__, msg);

    sprintf(msg, "ERROR: DISPLAY green LED is not OFF when the display is activated for the first time. Expected green level: %d, actual: %ld", 0, green_test);
    UNITY_TEST_ASSERT_UINT32_WITHIN(1, 0, green_test, __LINE__, msg);

    sprintf(msg, "ERROR: DISPLAY blue LED is not OFF when the display is activated for the first time. Expected blue level: %d, actual: %ld", 0, blue_test);
    UNITY_TEST_ASSERT_UINT32_WITHIN(1, 0, blue_test, __LINE__, msg);
}

/**
 * @brief Check the self-transition in SET_DISPLAY
 *
 */
void test_new_color(void)
{
    // Set state to SET_DISPLAY
    fsm_display_set_state(p_fsm_display, SET_DISPLAY);

    // Force status to active
    fsm_display_set_status(p_fsm_display, true);

    // Check the state assuming that the flag new_color is not set at the beginning
    // Check that the transition does not happen if the flag new_color is not set. Because it is a self-transition, instead of calling the fsm_fire, we do it directly
    fsm_t *p_fsm = fsm_display_get_inner_fsm(p_fsm_display);
    fsm_trans_t *p_t;
    for (p_t = p_fsm->p_tt; p_t->orig_state >= 0; ++p_t)
    {
        if ((p_fsm->current_state == p_t->orig_state) && p_t->in(p_fsm))
        {
            bool in_check_status = p_t->in(p_fsm);
            UNITY_TEST_ASSERT_EQUAL_INT(false, in_check_status, __LINE__, "The input condition function of the transition from SET_DISPLAY to SET_DISPLAY should return false if the new_color flag is not set");
        }
    }

    // Set state to SET_DISPLAY
    fsm_display_set_state(p_fsm_display, SET_DISPLAY);

    // Set an arbitrary distance and its arbitrary color
    uint32_t test_arbitrary_distance = (OK_MIN_CM + INFO_MIN_CM) / 2;
    uint8_t color_test_red = 25;
    uint8_t color_test_green = 89;
    uint8_t color_test_blue = 81;
    
    fsm_display_set_distance(p_fsm_display, test_arbitrary_distance);

    // Check that the self-transition happens, meaning that the new_color flag is set
    p_fsm = fsm_display_get_inner_fsm(p_fsm_display);

    for (p_t = p_fsm->p_tt; p_t->orig_state >= 0; ++p_t)
    {
        if ((p_fsm->current_state == p_t->orig_state) && p_t->in(p_fsm))
        {
            bool in_check_status = p_t->in(p_fsm);
            uint32_t state = fsm_display_get_state(p_fsm_display);

            // The transition should happen to the same state if the new_color flag is set. Check both conditions
            if (in_check_status)
            {
                p_t->out(p_fsm); // Call the output function for the sake of the test
            }

            // Check the state
            UNITY_TEST_ASSERT_EQUAL_INT(true, in_check_status, __LINE__, "The input condition function of the transition from SET_DISPLAY to SET_DISPLAY should return true if the new_color flag is set");
            UNITY_TEST_ASSERT_EQUAL_INT(SET_DISPLAY, state, __LINE__, "The FSM should remain in the SET_DISPLAY state if the new_color flag is set");
        }
    }

    // Check that it is idle and it is active
    bool is_active = fsm_display_get_status(p_fsm_display);
    bool idle_and_active = fsm_display_check_activity(p_fsm_display);
    UNITY_TEST_ASSERT_EQUAL_INT(true, is_active & !idle_and_active, __LINE__, "The FSM should be active and idle if the new_color flag is set");

    // Check that the color is set
    uint32_t arr = DISPLAY_RGB_PWM->ARR;
    uint32_t ccr_red = DISPLAY_RGB_PWM->CCR1;
    uint32_t ccr_green = DISPLAY_RGB_PWM->CCR3;
    uint32_t ccr_blue = DISPLAY_RGB_PWM->CCR4;

    // Given the distance set, compute the expected color
    uint32_t red_test = (ccr_red * TEST_PORT_DISPLAY_RGB_MAX_VALUE) / (arr + 1);
    uint32_t green_test = (ccr_green * TEST_PORT_DISPLAY_RGB_MAX_VALUE) / (arr + 1);
    uint32_t blue_test = (ccr_blue * TEST_PORT_DISPLAY_RGB_MAX_VALUE) / (arr + 1);

    sprintf(msg, "ERROR: DISPLAY red LED is not set to the correct color after setting a new distance. Expected red level: %d, actual: %ld", color_test_red, red_test);
    UNITY_TEST_ASSERT_UINT32_WITHIN(1, color_test_red, red_test, __LINE__, msg);

    sprintf(msg, "ERROR: DISPLAY green LED is not set to the correct color after setting a new distance. Expected green level: %d, actual: %ld", color_test_green, green_test);
    UNITY_TEST_ASSERT_UINT32_WITHIN(1, color_test_green, green_test, __LINE__, msg);

    sprintf(msg, "ERROR: DISPLAY blue LED is not set to the correct color after setting a new distance. Expected blue level: %d, actual: %ld", color_test_blue, blue_test);
    UNITY_TEST_ASSERT_UINT32_WITHIN(1, color_test_blue, blue_test, __LINE__, msg);
}

void test_check_off(void)
{
    // Set state to SET_DISPLAY
    fsm_display_set_state(p_fsm_display, SET_DISPLAY);
    
    // Check that the function returns true if the display is not active
    fsm_display_set_status(p_fsm_display, false);

    // Fire the transition
    fsm_display_fire(p_fsm_display);

    // Check the state
    fsm_t *p_inner_fsm = fsm_display_get_inner_fsm(p_fsm_display);
    UNITY_TEST_ASSERT_EQUAL_INT(WAIT_DISPLAY, fsm_get_state(p_inner_fsm), __LINE__, "The FSM should move to the WAIT_DISPLAY state if the display is not active");

    // Check that the color is OFF (CCxR are disabled)
    uint32_t ccer_red = DISPLAY_RGB_PWM->CCER & TIM_CCER_CC1E;
    uint32_t ccer_green = DISPLAY_RGB_PWM->CCER & TIM_CCER_CC3E;
    uint32_t ccer_blue = DISPLAY_RGB_PWM->CCER & TIM_CCER_CC4E;
    
    UNITY_TEST_ASSERT_EQUAL_UINT32(0, ccer_red, __LINE__, "The red LED should be disabled if the display is not active");
    UNITY_TEST_ASSERT_EQUAL_UINT32(0, ccer_green, __LINE__, "The green LED should be disabled if the display is not active");
    UNITY_TEST_ASSERT_EQUAL_UINT32(0, ccer_blue, __LINE__, "The blue LED should be disabled if the display is not active");

    // Check that it is not idle and it is not active
    bool is_active = fsm_display_get_status(p_fsm_display);
    bool idle_and_active = fsm_display_check_activity(p_fsm_display);
    UNITY_TEST_ASSERT_EQUAL_INT(false, is_active & !idle_and_active, __LINE__, "The FSM should not be active and not idle if the display is not active");    
}


int main(void)
{
    port_system_init();
    UNITY_BEGIN();

    RUN_TEST(test_initial_config);
    RUN_TEST(test_activation);
    RUN_TEST(test_new_color);
    RUN_TEST(test_check_off);

    exit(UNITY_END());
}
