cmake_minimum_required(VERSION 2.8.11)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)

#set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_RULE_MESSAGES OFF)

#set(DEVICE_NAME test_bt)
set(DEVICE_NAME timer_button_1x1)
#set(DEVICE_NAME fan_3x1_dht)
#set(DEVICE_NAME tensiometer_1x1)
#set(DEVICE_NAME pump_station_3x2)

### Atmega1284p
### Fuses: (E:FC, H:DE, L:FF)
set(BOARD uno_pro)

### Atmega328p
### Fuses: (E:FC, H:DC, L:FF)
#set(BOARD pro)

#set(CD_CO CONTROLLER_ONLY)

if(CD_CO)
project(${DEVICE_NAME}_${BOARD}_CO)
add_compile_definitions(${CD_CO})
else()
project(${DEVICE_NAME}_${BOARD})
endif()

set(${CMAKE_PROJECT_NAME}_BOARD ${BOARD})
set(${CMAKE_PROJECT_NAME}_SKETCH ${DEVICE_NAME}/poweroid_${DEVICE_NAME}.ino)

set(SDK_NAME Poweroid_SDK_10)
set(SDK_BUILD_PATH "${SDK_NAME}/cmake-build-release")

add_subdirectory(${SDK_NAME})

#### Uncomment below additional settings as needed.
set(${CMAKE_PROJECT_NAME}_PORT COM3)
set(${CMAKE_PROJECT_NAME}_PROGRAMMER arduinoisp)
file(GLOB_RECURSE LIB_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/${SDK_NAME}/lib/*.h)
file(GLOB_RECURSE SRC_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/${SDK_NAME}/src/*.h)

print_board_settings(${BOARD})

#set(LIB_HRDS ${CMAKE_CURRENT_SOURCE_DIR}/${SDK_NAME}/lib)
#set(SRC_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/${SDK_NAME}/src)

generate_arduino_firmware(${CMAKE_PROJECT_NAME} LIBS "${BOARD}_${SDK_NAME}" HDRS ${LIB_HDRS} ${SRC_HDRS})
