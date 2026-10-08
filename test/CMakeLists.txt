# Common unit tests (valid for all platforms)
FILE(GLOB TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ./test_*.c)
FOREACH(TEST_SOURCE ${TEST_SOURCES})
    # Rule to build unit tests
    GET_FILENAME_COMPONENT(TEST_NAME ${TEST_SOURCE} NAME_WE)
    ADD_EXECUTABLE(${TEST_NAME} ${TEST_SOURCE} ${PROJECT_PORT_ISR_SOURCES}) # TODO quitar ISR
    IF(DEFINED PLATFORM_EXTENSION)
        SET_TARGET_PROPERTIES(${TEST_NAME} PROPERTIES SUFFIX ${PLATFORM_EXTENSION})
    ENDIF()
    TARGET_LINK_LIBRARIES(${TEST_NAME} unity) # Link Unity test framework
    IF(PROJECT_COMMON_SOURCES)
        TARGET_LINK_LIBRARIES(${TEST_NAME} ${PROJECT_NAME}-common)
    ENDIF()
    TARGET_LINK_LIBRARIES(${TEST_NAME} ${PROJECT_NAME}-port)
    IF(USE_FSM)
        TARGET_LINK_LIBRARIES(${TEST_NAME} fsm)
    ENDIF()

    # Rules to run (native) or flash (OpenOCD) main executable
    IF(DEFINED OPENOCD_CONFIG_FILE)
        ADD_CUSTOM_TARGET(flash-${TEST_NAME}
            DEPENDS ${TEST_NAME}
            COMMAND ${OPENOCD_EXECUTABLE} -f ${OPENOCD_CONFIG_FILE} -c "program ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME}${PLATFORM_EXTENSION} verify reset exit"
            COMMENT "Flashing ${TEST_NAME}")
    ENDIF()
    IF(DEFINED QEMU_FLAGS)
        ADD_CUSTOM_TARGET(emulate-${TEST_NAME}
            DEPENDS ${TEST_NAME}
            COMMAND ${QEMU_EXECUTABLE} ${QEMU_FLAGS} -kernel ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME}${PLATFORM_EXTENSION}
            COMMENT "Emulating ${TEST_NAME}")
    ENDIF()
ENDFOREACH(TEST_SOURCE)

# Platform-specific unit tests (only valid for a specific platform)
FILE(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
FOREACH (child ${children})
    IF(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child})
        # assert that PLATFORM starts with the name of child directory
        STRING(FIND ${PLATFORM} ${child} PLATFORM_STARTS_WITH)
        IF(PLATFORM_STARTS_WITH EQUAL 0)
            # add test subdirectory if it exists
            ADD_SUBDIRECTORY(${child})
        ENDIF()
    ENDIF()
ENDFOREACH(child)
