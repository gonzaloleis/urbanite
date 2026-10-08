FILE(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
FOREACH (child ${children})
    IF(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child})
        # assert that PLATFORM starts with the name of child directory
        STRING(FIND ${PLATFORM} ${child} PLATFORM_STARTS_WITH)
        IF(PLATFORM_STARTS_WITH EQUAL 0)
            MESSAGE(STATUS "Adding platform-specific directory ${child}")
            ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/${child})
        ENDIF()
    ENDIF()
ENDFOREACH(child)

# Propagate platform-specific variables to parent scope
SET(PROJECT_PORT_ISR_SOURCES ${PROJECT_PORT_ISR_SOURCES} PARENT_SCOPE)  # TODO quitar
SET(PROJECT_PORT_SOURCES ${PROJECT_PORT_SOURCES} PARENT_SCOPE)
# For include directories, we add port/include to both port and common
SET(PROJECT_PORT_INCLUDE_DIRS ${PROJECT_PORT_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)
SET(PROJECT_COMMON_INCLUDE_DIRS ${PROJECT_COMMON_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)
