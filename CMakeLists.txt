cmake_minimum_required(VERSION 3.14.3)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)

#set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_RULE_MESSAGES OFF)

#set(DEVICE_NAME test_bt)
#set(DEVICE_NAME timer_button_1x1)
#set(DEVICE_NAME fan_3x1_dht)
#set(DEVICE_NAME thermo_1-2x2_dht)
#set(DEVICE_NAME tensiometer_1x1)
set(DEVICE_NAME pump_station_3x2)


#########
# Controler boards section
#
### Atmega1284p
### Fuses: (E:FC, H:DE, L:FF)
set(BOARD uno_pro)
#
### Atmega328p
### Fuses: (E:FC, H:DC, L:FF)
#set(BOARD pro)
#
#########


#### Uncomment below additional settings as needed.

#set(CD_CTRL CONTROLLER_ONLY)
#set(CD_CTRL NO_CONTROLLER)

#set(PWR_BOARD_SIZE MINI)
set(PWR_BOARD_VERSIONS PWR_BOARD_VERSION=PWR23)
set(RAM SAVE_RAM)
#set(STATES ARMED)

add_compile_definitions(${CD_CTRL} ${PWR_BOARD_SIZE} ${PWR_BOARD_VERSIONS} ${RAM} ${STATES})

if(CD_CTRL)
    if(${CD_CTRL} STREQUAL "CONTROLLER_ONLY")
        set(PRJ_NAME "${DEVICE_NAME}_${BOARD}_CO")
    elseif(${CD_CTRL} STREQUAL "NO_CONTROLLER")
        set(PRJ_NAME "${DEVICE_NAME}_${BOARD}_NC")
    endif()
    if(PWR_BOARD_SIZE)
        set(PRJ_NAME "${PRJ_NAME}_${PWR_BOARD_SIZE}")
    endif()
    project(${PRJ_NAME})
else()
    project(${DEVICE_NAME}_${BOARD})
endif()

set(${CMAKE_PROJECT_NAME}_BOARD ${BOARD})
set(${CMAKE_PROJECT_NAME}_SKETCH projects/${DEVICE_NAME}/poweroid_${DEVICE_NAME}.ino)

set(SDK_NAME Poweroid_SDK_10)
set(SDK_BUILD_PATH "${SDK_NAME}/cmake-build-release")

add_subdirectory(${SDK_NAME})

set(${CMAKE_PROJECT_NAME}_PORT COM3)
set(${CMAKE_PROJECT_NAME}_PROGRAMMER arduinoisp)

file(GLOB_RECURSE LIB_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/${SDK_NAME}/lib/*.h)
file(GLOB_RECURSE SRC_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/${SDK_NAME}/src/*.h)
file(GLOB_RECURSE SKETCH_HDRS projects/${DEVICE_NAME}/*.h)

message(STATUS "Compile definitions  ${COMPILE_DEFINITIONS}")

message(STATUS "Board settings")
print_board_settings(${BOARD})

generate_arduino_firmware(${CMAKE_PROJECT_NAME} LIBS "${BOARD}_${SDK_NAME}" HDRS ${LIB_HDRS} ${SRC_HDRS} ${SKETCH_HDRS})
