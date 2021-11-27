cmake_minimum_required(VERSION 3.19)
set(CMAKE_TOOLCHAIN_FILE cmake/ArduinoToolchain.cmake)

#set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_RULE_MESSAGES OFF)

#set(DEVICE_NAME blink)
#set(DEVICE_NAME test)

set(DEVICE_NAME timer_button_1x1)
#set(DEVICE_NAME fan_3x1_dht)
#set(DEVICE_NAME thermo_1-2x2_dht)
#set(DEVICE_NAME tensiometer_1x1)
#set(DEVICE_NAME repeater_0x0_ss)
#set(DEVICE_NAME level_controller_1x1_us_t)
#set(DEVICE_NAME pump_station_3x2)
#set(DEVICE_NAME meteo_2_dht)

#########
# Controller boards section
#
### Atmega1284p
### Fuses: (E:FC, H:DE, L:FF)
set(BOARD uno_pro)
#
#
### Atmega328p
### Fuses: (E:FC, H:DC, L:FF)
#set(BOARD pro)
#
#########


#### Uncomment below additional settings as needed

set(PWR_BOARD_VERSION PWR_BOARD_VERSION=PWR23)

#set(PWR_BOARD_SIZE MINI)

#### Software Serial definitions
#set(IO_SSERIAL SSERIAL)
#set(IO_SERIAL_BUFFER_LEN 128)
#add_compile_definitions(S_BUFF_LEN=${IO_SERIAL_BUFFER_LEN} SERIAL_TX_BUFFER_SIZE=S_BUFF_LEN SERIAL_RX_BUFFER_SIZE=S_BUFF_LEN _SS_MAX_RX_BUFF=S_BUFF_LEN)

#set(CD_CTRL CONTROLLER_ONLY)
set(CD_CTRL NO_CONTROLLER)
#set(CD_CTRL I2C_CONTROLLER)

#set(RAM SAVE_RAM)
set(REMOTE DO_NOT_PARSE_REMOTE)
#set(TOKEN ALLOW_TOKEN)

#set(RTCM RTCM)
#set(STATES STATES_ARMED)

#Set logging output DEBUG or INFO
#add_compile_definitions(DEBUG)

#Add this to the compile definitions in order to redefine power pins
#add_compile_definitions(PWR1_PIN=9)

add_compile_definitions(${CD_CTRL} ${PWR_BOARD_SIZE} ${PWR_BOARD_VERSIONS} ${RAM} ${IO_SSERIAL} ${STATES} ${RTCM} ${REMOTE} ${TOKEN} __STDC_LIMIT_MACROS )

set(PROJECT_FOLDER "devices/${DEVICE_NAME}")
string(TOUPPER ${BOARD} BOARD_NAME)
if(PWR_BOARD_SIZE)
    set(BOARD_NAME "${BOARD_NAME}_${PWR_BOARD_SIZE}")
endif()

if(RTCM)
    set(BOARD_NAME "${BOARD_NAME}_${RTCM}")
endif()

if(CD_CTRL)
    if(${CD_CTRL} STREQUAL "CONTROLLER_ONLY")
        set(PRJ_NAME "${DEVICE_NAME}_${BOARD_NAME}_CO")
    elseif(${CD_CTRL} STREQUAL "NO_CONTROLLER")
        set(PRJ_NAME "${DEVICE_NAME}_${BOARD_NAME}_NC")
    elseif(${CD_CTRL} STREQUAL "I2C_CONTROLLER")
        set(PRJ_NAME "${DEVICE_NAME}_${BOARD_NAME}_CI2C")
    endif()
    project(${PRJ_NAME} LANGUAGES C CXX ASM)
else()
    project(${DEVICE_NAME}_${BOARD_NAME} LANGUAGES C CXX ASM)
endif()

set(${CMAKE_PROJECT_NAME}_PORT COM3)
set(${CMAKE_PROJECT_NAME}_PROGRAMMER arduinoisp)

set(${CMAKE_PROJECT_NAME}_BOARD ${BOARD})
set(${CMAKE_PROJECT_NAME}_SKETCH ${PROJECT_FOLDER}/poweroid_${DEVICE_NAME}.ino)

set(SDK_NAME poweroid-sdk-1.0)

include_directories(AFTER ${SDK_NAME}/lib)
include_directories(AFTER ${SDK_NAME}/src)
add_subdirectory(${SDK_NAME})

file(GLOB_RECURSE LIB_HDRS ${SDK_NAME}/lib/*.h)
file(GLOB_RECURSE SRC_HDRS ${SDK_NAME}/src/*.h)
file(GLOB_RECURSE SKETCH_HDRS ${PROJECT_FOLDER}/*.h)

message(STATUS "Compile definitions  ${COMPILE_DEFINITIONS}")

message(STATUS "Board settings")
print_board_settings(${BOARD})

generate_arduino_firmware(${CMAKE_PROJECT_NAME} LIBS "${BOARD}_${SDK_NAME}" HDRS ${SKETCH_HDRS})
