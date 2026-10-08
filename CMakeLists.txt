SET(PROJECT_NAME project) # set it to your project name

IF(NOT DEFINED VERBOSE)
    SET(VERBOSE true) # set it to true to print additional information
    MESSAGE(STATUS "No verbosity selected, using default (${VERBOSE}). You can override it by passing -DVERBOSE=<verbose> to cmake")
ENDIF()
IF(NOT DEFINED MATRIXMCU)
    SET(MATRIXMCU  ${CMAKE_CURRENT_SOURCE_DIR}/../..) # set it to your default path to MatrixMCU directory
    MESSAGE(STATUS "No path to MatrixMCU directory selected, using default (${MATRIXMCU}). You can override it by passing -DMATRIXMCU=<MATRIXMCU> to cmake")
ENDIF()
IF(NOT DEFINED USE_FSM)
    SET(USE_FSM true) # set it to true to use FSM library by default
    MESSAGE(STATUS "No FSM library usage selected, using default (${USE_FSM}). You can override it by passing -DUSE_FSM=<use_fsm> to cmake")
ENDIF()
IF(NOT DEFINED USE_HAL)
    SET(USE_HAL false) # set it to true to use HAL library by default
    MESSAGE(STATUS "No HAL library usage selected, using default (${USE_HAL}). You can override it by passing -DUSE_HAL=<use_hal> to cmake")
ENDIF()
IF(NOT DEFINED PLATFORM)
    SET(PLATFORM "stm32f446re") # PORTABILITY: change this to your platform
    MESSAGE(STATUS "No platform selected, using default (${PLATFORM}). You can override it by passing -DPLATFORM=<platform> to cmake")
ENDIF()
IF(NOT DEFINED CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug) # set it to your default build type
    MESSAGE(STATUS "No build type selected, using default (${CMAKE_BUILD_TYPE}). You can override it by passing -DCMAKE_BUILD_TYPE=<build_type> to cmake")
ENDIF()
IF (NOT DEFINED USE_SEMIHOSTING)
    SET(USE_SEMIHOSTING true)
    MESSAGE(STATUS "Semihosting not specified, using default (${USE_SEMIHOSTING}). You can override it by passing -DUSE_SEMIHOSTING=<use_semihosting> to cmake")
ENDIF()

########################################################################################
## IF YOU DON'T KNOW WHAT YOU ARE DOING, DO **NOT** EDIT THIS FILE FROM THIS POINT ON ##
########################################################################################

# Load platform-specific setup configuration (e.g., toolchain and libraries)
INCLUDE(${MATRIXMCU}/CMakeLists.txt)

# CMake project configuration
CMAKE_MINIMUM_REQUIRED(VERSION 3.24)
PROJECT(${PROJECT_NAME} C ASM)
SET(CMAKE_C_STANDARD 11)

# Add platform-agnostic flags
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -Wno-unused-parameter")

# Build type-specific flags
SET(CMAKE_C_FLAGS_DEBUG "-g -O0")
SET(CMAKE_C_FLAGS_RELEASE "-O3")

# Set output directory for binaries
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/${PLATFORM}/${CMAKE_BUILD_TYPE})

# Add configuration-specific compile definitions
IF (USE_SEMIHOSTING)
    add_compile_definitions(USE_SEMIHOSTING)
ENDIF()

# Find source and include files of the project
ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/common)  # load project library configuration (common)
FILE(GLOB PROJECT_COMMON_SOURCES ${PROJECT_COMMON_SOURCES})  # project library source files

ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/port)    # load project library configuration (port)
FILE(GLOB PROJECT_PORT_SOURCES ${PROJECT_PORT_SOURCES})  # project library source files
IF(DEFINED PROJECT_PORT_ISR_SOURCES) # TODO quitar
    FILE(GLOB PROJECT_PORT_ISR_SOURCES ${PROJECT_PORT_ISR_SOURCES}) # project library ISR source files
ENDIF()

IF(VERBOSE)
    MESSAGE(STATUS "Found common include directories: ${PROJECT_COMMON_INCLUDE_DIRS}")  
    MESSAGE(STATUS "Found port include directories: ${PROJECT_PORT_INCLUDE_DIRS}")
    MESSAGE(STATUS "Found common source files: ${PROJECT_COMMON_SOURCES}")
    MESSAGE(STATUS "Found port source files: ${PROJECT_PORT_SOURCES}")
    IF(DEFINED PROJECT_PORT_ISR_SOURCES) # TODO quitar ISR
        MESSAGE(STATUS "Found port ISR source files: ${PROJECT_PORT_ISR_SOURCES}")
    ENDIF()
ENDIF()

# Create project libraries
IF(PROJECT_COMMON_SOURCES)
    ADD_LIBRARY(${PROJECT_NAME}-common STATIC)
    TARGET_SOURCES(${PROJECT_NAME}-common PRIVATE ${PROJECT_COMMON_SOURCES})
    TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}-common PUBLIC ${PROJECT_COMMON_INCLUDE_DIRS})
    # link FSM library to project library (if applies)
    IF(USE_FSM)
        TARGET_LINK_LIBRARIES(${PROJECT_NAME}-common fsm) 
    ENDIF()
ENDIF()

ADD_LIBRARY(${PROJECT_NAME}-port STATIC)
TARGET_SOURCES(${PROJECT_NAME}-port PRIVATE ${PLATFORM_SOURCES} ${PLATFORM_HAL_SOURCES} ${PROJECT_PORT_SOURCES})
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}-port PUBLIC ${PROJECT_PORT_INCLUDE_DIRS} ${PLATFORM_INCLUDE_DIRS} ${PLATFORM_HAL_INCLUDE_DIRS})

# Rules to build main executable

FILE(GLOB PROJECT_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/main.c) # project main routine
# ADD_EXECUTABLE(main ${PROJECT_MAIN})
ADD_EXECUTABLE(main ${PROJECT_MAIN} ${PROJECT_PORT_ISR_SOURCES}) # TODO quitar ISR
IF(DEFINED PLATFORM_EXTENSION)
    SET_TARGET_PROPERTIES(main PROPERTIES SUFFIX ${PLATFORM_EXTENSION})
ENDIF()
IF(PROJECT_COMMON_SOURCES)
    TARGET_LINK_LIBRARIES(main ${PROJECT_NAME}-common)
ENDIF()
TARGET_LINK_LIBRARIES(main ${PROJECT_NAME}-port)
IF(USE_FSM)
    TARGET_LINK_LIBRARIES(main fsm)
ENDIF()

# Rules to flash (OpenOCD)
IF(DEFINED OPENOCD_CONFIG_FILE)
    ADD_CUSTOM_TARGET(flash-main
        DEPENDS main
        COMMAND ${OPENOCD_EXECUTABLE} -f ${OPENOCD_CONFIG_FILE} -c "program ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/main${PLATFORM_EXTENSION} verify reset exit"
        COMMENT "Flashing main")
ENDIF()
# Rules to emulate (QEMU)
IF(DEFINED QEMU_FLAGS)
    ADD_CUSTOM_TARGET(emulate-main
        DEPENDS main
        COMMAND ${QEMU_EXECUTABLE} ${QEMU_FLAGS} -kernel ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/main${PLATFORM_EXTENSION}
        COMMENT "Emulating main")
ENDIF()

# Add tests
ADD_SUBDIRECTORY(test)
# Add examples
ADD_SUBDIRECTORY(example)
