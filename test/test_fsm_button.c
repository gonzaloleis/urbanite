/**
 * @file test_fsm_button.c
 * @brief Unit test for the button FSM.
 *
 * @author Sistemas Digitales II
 * @date 2025-01-01
 */
/* System dependent libraries */
#include <stdlib.h>
#include <unity.h>

/* HW independent libraries */
#include "port_button.h"
#include "port_system.h"
#include "stm32f4_system.h"
#include "stm32f4_button.h"

/* Include FSM libraries */
#include "fsm.h"
#include "fsm_button.h"

/* Defines */
#define USER_BUTTON_DEBOUNCE_TIME_MS 150 /*!< Button debounce time in milliseconds */

static fsm_button_t *p_fsm_button;

void setUp(void)
{
    p_fsm_button = fsm_button_new(USER_BUTTON_DEBOUNCE_TIME_MS, PORT_PARKING_BUTTON_ID);
    port_button_disable_interrupts(PORT_PARKING_BUTTON_ID); // Disable EXTI to avoid unwanted interrupts
}

void tearDown(void)
{
    // Nothing to do
}

void test_initial_config(void)
{
    fsm_t *p_inner_fsm = fsm_button_get_inner_fsm(p_fsm_button);
    UNITY_TEST_ASSERT_EQUAL_PTR(p_fsm_button, p_inner_fsm, __LINE__, "The inner FSM of fsm_button_t is not the first field of the struct");

    UNITY_TEST_ASSERT_EQUAL_INT(BUTTON_RELEASED, fsm_get_state(p_inner_fsm), __LINE__, "The initial state of the FSM is not BUTTON_RELEASED");

    // It assumes there are 4 transitions in the table plus the null transition
    fsm_trans_t *last_transition = &p_inner_fsm->p_tt[4];

    UNITY_TEST_ASSERT_EQUAL_INT(-1, last_transition->orig_state, __LINE__, "The origin state of the last transition of the FSM should be -1");
    UNITY_TEST_ASSERT_EQUAL_INT(NULL, last_transition->in, __LINE__, "The input condition function of the last transition of the FSM should be NULL");
    UNITY_TEST_ASSERT_EQUAL_INT(-1, last_transition->dest_state, __LINE__, "The destination state of the last transition of the FSM should be -1");
    UNITY_TEST_ASSERT_EQUAL_INT(NULL, last_transition->out, __LINE__, "The output modification function of the last transition of the FSM should be NULL");
}

void _test_button_press(uint32_t press_time)
{
    // First transition
    port_button_set_pressed(PORT_PARKING_BUTTON_ID, true);

    fsm_button_fire(p_fsm_button);
    UNITY_TEST_ASSERT_EQUAL_INT(BUTTON_PRESSED_WAIT, fsm_button_get_state(p_fsm_button), __LINE__, "The FSM did not change to BUTTON_PRESSED_WAIT after pressing the button");

    // Second transition
    port_system_delay_ms(press_time);
    fsm_button_fire(p_fsm_button);
    uint32_t debounce_time_ms = fsm_button_get_debounce_time_ms(p_fsm_button);

    if (press_time < debounce_time_ms)
    {
        UNITY_TEST_ASSERT_EQUAL_INT(BUTTON_PRESSED_WAIT, fsm_button_get_state(p_fsm_button), __LINE__, "The FSM did not change to BUTTON_RELEASED_WAIT after pressing the button for a short time");
    }
    else
    {
        UNITY_TEST_ASSERT_EQUAL_INT(BUTTON_PRESSED, fsm_button_get_state(p_fsm_button), __LINE__, "The FSM did not change to BUTTON_PRESSED after pressing the button for a long time");

        // Third transition
        port_button_set_pressed(PORT_PARKING_BUTTON_ID, false);
        
        fsm_button_fire(p_fsm_button);
        UNITY_TEST_ASSERT_EQUAL_INT(BUTTON_RELEASED_WAIT, fsm_button_get_state(p_fsm_button), __LINE__, "The FSM did not change to BUTTON_RELEASED_WAIT after releasing the button");

        // Fourth transition
        debounce_time_ms = fsm_button_get_debounce_time_ms(p_fsm_button);

        port_system_delay_ms(debounce_time_ms + 1);
        fsm_button_fire(p_fsm_button);
        UNITY_TEST_ASSERT_EQUAL_INT(BUTTON_RELEASED, fsm_button_get_state(p_fsm_button), __LINE__, "The FSM did not change to BUTTON_RELEASED after releasing the button");
    }
}

void test_short_button_press(void)
{
    _test_button_press(100);
}

void test_long_button_press(void)
{
    _test_button_press(1000);
}

int main(void)
{
    port_system_init();
    UNITY_BEGIN();

    RUN_TEST(test_initial_config);
    RUN_TEST(test_short_button_press);
    RUN_TEST(test_long_button_press);

    exit(UNITY_END());
}
