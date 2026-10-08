# Project library headers
SET(PROJECT_PORT_INCLUDE_DIRS ${PROJECT_PORT_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)
# Project library sources
SET(PROJECT_PORT_SOURCES ${PROJECT_PORT_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c PARENT_SCOPE)


# Project ISR sources must be added manually to avoid the linker to optimize them out TODO quitar
SET(PROJECT_PORT_ISR_SOURCES ${PROJECT_PORT_ISR_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src/interr.c PARENT_SCOPE)
