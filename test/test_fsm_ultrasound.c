/**
 * @file test_fsm_ultrasound.c
 * @brief Unit test for the ultrasound FSM.
 *
 * @author Sistemas Digitales II
 * @date 2025-01-01
 */
/* System dependent libraries */
#include <stdlib.h>
#include <unity.h>

/* HW independent libraries */
#include "port_ultrasound.h"
#include "port_system.h"
#include "stm32f4_system.h"
#include "stm32f4_ultrasound.h"

/* Include FSM libraries */
#include "fsm.h"
#include "fsm_ultrasound.h"

/* Defines and enums ----------------------------------------------------------*/
/* Defines */
#define PORT_REAR_PARKING_SENSOR_ID 0 /*!< Ultrasound identifier @hideinitializer */

// Trigger timer configuration
#define REAR_TRIGGER_TIMER TIM3 /*!< Trigger signal timer @hideinitializer */
#define REAR_ECHO_TIMER TIM2    /*!< Echo signal timer @hideinitializer */
#define MEASUREMENT_TIMER TIM5  /*!< Ultrasound measurement timer @hideinitializer */

/* Global variables ----------------------------------------------------------*/
static char msg[200];                      /*!< Buffer for the error messages */
static fsm_ultrasound_t *p_fsm_ultrasound; /*!< Pointer to the ultrasound FSM */

/* Private functions ---------------------------------------------------------*/
void setUp(void)
{
    p_fsm_ultrasound = fsm_ultrasound_new(PORT_REAR_PARKING_SENSOR_ID);
}

void tearDown(void)
{
    // Nothing to do
}

/**
 * @brief Test the configuration of the ultrasound FSM.
 *
 */
void test_initial_config(void)
{
    fsm_t *p_inner_fsm = fsm_ultrasound_get_inner_fsm(p_fsm_ultrasound);
    UNITY_TEST_ASSERT_EQUAL_PTR(p_fsm_ultrasound, p_inner_fsm, __LINE__, "The inner FSM of fsm_ultrasound_t is not the first field of the struct");

    UNITY_TEST_ASSERT_EQUAL_INT(WAIT_START, fsm_get_state(p_inner_fsm), __LINE__, "The initial state of the FSM is not WAIT_START");

    // It assumes there are 6 transitions in the table plus the null transition
    fsm_trans_t *last_transition = &p_inner_fsm->p_tt[6];

    UNITY_TEST_ASSERT_EQUAL_INT(-1, last_transition->orig_state, __LINE__, "The origin state of the last transition of the FSM should be -1");
    UNITY_TEST_ASSERT_EQUAL_INT(NULL, last_transition->in, __LINE__, "The input condition function of the last transition of the FSM should be NULL");
    UNITY_TEST_ASSERT_EQUAL_INT(-1, last_transition->dest_state, __LINE__, "The destination state of the last transition of the FSM should be -1");
    UNITY_TEST_ASSERT_EQUAL_INT(NULL, last_transition->out, __LINE__, "The output modification function of the last transition of the FSM should be NULL");
}

/**
 * @brief Test the start of a new measurement.
 *
 */
void test_start_measurement(void)
{
    // Set both flags to true
    port_ultrasound_set_trigger_ready(PORT_REAR_PARKING_SENSOR_ID, true);
    fsm_ultrasound_set_status(p_fsm_ultrasound, true);

    // Check the transition
    fsm_ultrasound_fire(p_fsm_ultrasound);
    UNITY_TEST_ASSERT_EQUAL_INT(TRIGGER_START, fsm_ultrasound_get_state(p_fsm_ultrasound), __LINE__, "The FSM did not change to TRIGGER_START after indicating the start of a measurement");
}

void test_trigger_end(void)
{
    port_ultrasound_set_trigger_end(PORT_REAR_PARKING_SENSOR_ID, true);

    // Set the state to TRIGGER_START
    fsm_ultrasound_set_state(p_fsm_ultrasound, TRIGGER_START);

    // Check the transition
    fsm_ultrasound_fire(p_fsm_ultrasound);
    UNITY_TEST_ASSERT_EQUAL_INT(WAIT_ECHO_START, fsm_ultrasound_get_state(p_fsm_ultrasound), __LINE__, "The FSM did not change to WAIT_ECHO_START from TRIGGER_START after indicating the end of the trigger signal");

    // Check that the trigger pin is low and the trigger timer is disabled
    bool trigger_end = port_ultrasound_get_trigger_end(PORT_REAR_PARKING_SENSOR_ID);

    UNITY_TEST_ASSERT_EQUAL_UINT32(false, trigger_end, __LINE__, "The trigger pin should be lowered after the trigger signal has ended in the transition from TRIGGER_START to WAIT_ECHO_START");

    // Check that the trigger timer is disabled
    uint32_t tim_trigger_en = (REAR_TRIGGER_TIMER->CR1) & TIM_CR1_CEN_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(false, tim_trigger_en, __LINE__, "The trigger timer should be disabled after the trigger signal has ended in the transition from TRIGGER_START to WAIT_ECHO_START");
}

void test_echo_init(void)
{
    port_ultrasound_set_echo_init_tick(PORT_REAR_PARKING_SENSOR_ID, 1);

    // Set the state to WAIT_ECHO_START
    fsm_ultrasound_set_state(p_fsm_ultrasound, WAIT_ECHO_START);

    // Check the transition
    fsm_ultrasound_fire(p_fsm_ultrasound);
    UNITY_TEST_ASSERT_EQUAL_INT(WAIT_ECHO_END, fsm_ultrasound_get_state(p_fsm_ultrasound), __LINE__, "The FSM did not change to WAIT_ECHO_END from WAIT_ECHO_START after receiving the echo init tick");

    // Set an invalid echo init tick
    port_ultrasound_set_echo_init_tick(PORT_REAR_PARKING_SENSOR_ID, 0);

    // Set the state to WAIT_ECHO_START
    fsm_ultrasound_set_state(p_fsm_ultrasound, WAIT_ECHO_START);

    // Check the transition that should not take place
    fsm_ultrasound_fire(p_fsm_ultrasound);
    UNITY_TEST_ASSERT_EQUAL_INT(WAIT_ECHO_START, fsm_ultrasound_get_state(p_fsm_ultrasound), __LINE__, "The FSM changed from WAIT_ECHO_START to WAIT_ECHO_END after receiving an invalid echo init tick");
}

void test_echo_received_and_distance(void)
{
    uint32_t init_ticks[FSM_ULTRASOUND_NUM_MEASUREMENTS] = {1, 64371, 3, 63208, 5};
    uint32_t end_ticks[FSM_ULTRASOUND_NUM_MEASUREMENTS] = {584, 3, 1752, 4, 2920};
    uint32_t overflows[FSM_ULTRASOUND_NUM_MEASUREMENTS] = {0, 1, 0, 1, 0};
    uint32_t expected_time_diff_ticks[FSM_ULTRASOUND_NUM_MEASUREMENTS] = {583, 1168, 1749, 2332, 2915};
    uint32_t expected_distance[FSM_ULTRASOUND_NUM_MEASUREMENTS] = {10, 20, 30, 40, 50};
    uint32_t expected_median = 30;

    // Set some values to the echo signal ticks
    for (uint32_t i = 0; i < FSM_ULTRASOUND_NUM_MEASUREMENTS; i++)
    {
        // Set the state to WAIT_ECHO_END
        fsm_ultrasound_set_state(p_fsm_ultrasound, WAIT_ECHO_END); // Avoids jumping to the next state

        port_ultrasound_stop_ultrasound(PORT_REAR_PARKING_SENSOR_ID); // Avoid unwanted interrupts
        port_ultrasound_set_echo_received(PORT_REAR_PARKING_SENSOR_ID, true);
        port_ultrasound_set_echo_init_tick(PORT_REAR_PARKING_SENSOR_ID, init_ticks[i]);
        port_ultrasound_set_echo_end_tick(PORT_REAR_PARKING_SENSOR_ID, end_ticks[i]);
        port_ultrasound_set_echo_overflows(PORT_REAR_PARKING_SENSOR_ID, overflows[i]);

        printf("Init tick: %ld, End tick: %ld, Overflows: %ld.\n\tExpected time diff: %ld ticks, Expected distance: %ld cm.\n", init_ticks[i], end_ticks[i], overflows[i], expected_time_diff_ticks[i], expected_distance[i]);

        // Check the transition
        fsm_ultrasound_fire(p_fsm_ultrasound);
        UNITY_TEST_ASSERT_EQUAL_INT(SET_DISTANCE, fsm_ultrasound_get_state(p_fsm_ultrasound), __LINE__, "The FSM did not change to SET_DISTANCE from WAIT_ECHO_END after receiving the echo signal");

        // Check that the echo signal is cleared
        bool echo_received = port_ultrasound_get_echo_received(PORT_REAR_PARKING_SENSOR_ID);
        UNITY_TEST_ASSERT_EQUAL_UINT32(false, echo_received, __LINE__, "The echo signal should be cleared after the transition from WAIT_ECHO_END to SET_DISTANCE");
    }

    // Check that the distance is correctly set and the index is corretly updated
    uint32_t distance = fsm_ultrasound_get_distance(p_fsm_ultrasound);

    // Calculate distance in cm taking into account the speed of sound (1cm = 58.3us)
    sprintf(msg, "ERROR: The median distance is not correctly set after the transition from WAIT_ECHO_END to SET_DISTANCE. The error is higher than 1cm");
    UNITY_TEST_ASSERT_INT_WITHIN(1, expected_median, distance, __LINE__, msg);

    // Repeat the test to check that the final distance is not computed as moving median, but when the buffer is full. Set the first distances to 0
    uint32_t mid_idx = (FSM_ULTRASOUND_NUM_MEASUREMENTS % 2 == 0) ? (FSM_ULTRASOUND_NUM_MEASUREMENTS / 2) + 1 : (FSM_ULTRASOUND_NUM_MEASUREMENTS / 2);

    for (uint32_t i = 0; i <= mid_idx; i++)
    {
        // Set the state to WAIT_ECHO_END
        fsm_ultrasound_set_state(p_fsm_ultrasound, WAIT_ECHO_END); // Avoids jumping to the next state

        port_ultrasound_set_echo_received(PORT_REAR_PARKING_SENSOR_ID, true);
        port_ultrasound_set_echo_init_tick(PORT_REAR_PARKING_SENSOR_ID, 0);
        port_ultrasound_set_echo_end_tick(PORT_REAR_PARKING_SENSOR_ID, 0);
        port_ultrasound_set_echo_overflows(PORT_REAR_PARKING_SENSOR_ID, 0);
        fsm_ultrasound_fire(p_fsm_ultrasound);
    }

    // Check that the distance is correctly set
    distance = fsm_ultrasound_get_distance(p_fsm_ultrasound);

    sprintf(msg, "ERROR: The median distance is being computed before the buffer is full, i.e. before the index is equal to the number of %d", FSM_ULTRASOUND_NUM_MEASUREMENTS); 
    UNITY_TEST_ASSERT_INT_WITHIN(1, expected_median, distance, __LINE__, msg);
}

/**
 * @brief Check the transition from SET_DISTANCE to TRIGGER_START
 *
 */
void test_new_measurement(void)
{
    // Set the state to SET_DISTANCE
    fsm_ultrasound_set_state(p_fsm_ultrasound, SET_DISTANCE);

    // Set the trigger ready flag to true
    port_ultrasound_set_trigger_ready(PORT_REAR_PARKING_SENSOR_ID, true);

    // Check the transition
    fsm_ultrasound_fire(p_fsm_ultrasound);
    UNITY_TEST_ASSERT_EQUAL_INT(TRIGGER_START, fsm_ultrasound_get_state(p_fsm_ultrasound), __LINE__, "The FSM did not change to TRIGGER_START from SET_DISTANCE after indicating a new measurement is ready");
}

/**
 * @brief Check the transition from SET_DISTANCE to WAIT_START
 *
 */
void test_stop_measurement(void)
{
    // Set the trigger ready flag to false
    port_ultrasound_set_trigger_ready(PORT_REAR_PARKING_SENSOR_ID, false);

    // Set the state to SET_DISTANCE
    fsm_ultrasound_set_state(p_fsm_ultrasound, SET_DISTANCE);

    // Set the status flag to false
    fsm_ultrasound_set_status(p_fsm_ultrasound, false);

    // Check the transition
    fsm_ultrasound_fire(p_fsm_ultrasound);
    UNITY_TEST_ASSERT_EQUAL_INT(WAIT_START, fsm_ultrasound_get_state(p_fsm_ultrasound), __LINE__, "The FSM did not change to WAIT_START from SET_DISTANCE after stopping the measurement");

    // Check that all the timers have been disabled
    uint32_t tim_trigger_en = (REAR_TRIGGER_TIMER->CR1) & TIM_CR1_CEN_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(false, tim_trigger_en, __LINE__, "The trigger timer should be disabled after stopping the measurement");

    uint32_t tim_echo_en = (REAR_ECHO_TIMER->CR1) & TIM_CR1_CEN_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(false, tim_echo_en, __LINE__, "The echo timer should be disabled after stopping the measurement");

    uint32_t tim_meas_en = (MEASUREMENT_TIMER->CR1) & TIM_CR1_CEN_Msk;
    UNITY_TEST_ASSERT_EQUAL_UINT32(false, tim_meas_en, __LINE__, "The measurement timer should be disabled after stopping the measurement");

    // Check that all the ticks have been reset
    uint32_t echo_init_tick = port_ultrasound_get_echo_init_tick(PORT_REAR_PARKING_SENSOR_ID);
    UNITY_TEST_ASSERT_EQUAL_UINT32(0, echo_init_tick, __LINE__, "The echo init tick should be reset after stopping the measurement");

    uint32_t echo_end_tick = port_ultrasound_get_echo_end_tick(PORT_REAR_PARKING_SENSOR_ID);
    UNITY_TEST_ASSERT_EQUAL_UINT32(0, echo_end_tick, __LINE__, "The echo end tick should be reset after stopping the measurement");

    uint32_t echo_overflows = port_ultrasound_get_echo_overflows(PORT_REAR_PARKING_SENSOR_ID);
    UNITY_TEST_ASSERT_EQUAL_UINT32(0, echo_overflows, __LINE__, "The echo overflows should be reset after stopping the measurement");

    // Check that the echo signal is cleared
    bool echo_received = port_ultrasound_get_echo_received(PORT_REAR_PARKING_SENSOR_ID);
    UNITY_TEST_ASSERT_EQUAL_UINT32(false, echo_received, __LINE__, "The echo signal should be cleared after stopping the measurement");
}

int main(void)
{
    port_system_init();
    UNITY_BEGIN();

    RUN_TEST(test_initial_config);
    RUN_TEST(test_start_measurement);
    RUN_TEST(test_trigger_end);
    RUN_TEST(test_echo_received_and_distance);
    RUN_TEST(test_new_measurement);
    RUN_TEST(test_stop_measurement);
    exit(UNITY_END());
}
