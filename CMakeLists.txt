cmake_minimum_required(VERSION 2.8.11)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)

#set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_RULE_MESSAGES OFF)

#set(DEVICE_NAME timer_button_1x1)
#set(DEVICE_NAME fan_3x1_dht)
set(DEVICE_NAME pump_station_3x2)

#set(BOARD uno_pro)
set(BOARD pro)

project(${DEVICE_NAME}_${BOARD})

set(${CMAKE_PROJECT_NAME}_BOARD ${BOARD})
set(${CMAKE_PROJECT_NAME}_SKETCH ${DEVICE_NAME}/poweroid_${DEVICE_NAME}.ino)

set(SDK_NAME Poweroid_SDK_10)
set(SDK_BUILD_PATH "${SDK_NAME}/cmake-build-release")

#find_library(SDK_LIB "${BOARD}_${SDK_NAME}" PATHS "${SDK_BUILD_PATH}" NO_DEFAULT_PATH)
#message(STATUS "SDK library found: ${SDK_LIB}")

#find_library(MC_LIB "${BOARD}_MultiClick" PATHS "${SDK_BUILD_PATH}" NO_DEFAULT_PATH)
add_subdirectory(${SDK_NAME})

### Fuses: (E:FC, H:DE, L:FF)
#### Uncomment below additional settings as needed.
set(${CMAKE_PROJECT_NAME}_PORT COM3)
set(${CMAKE_PROJECT_NAME}_PROGRAMMER arduinoisp)
# set(pro.build.mcu atmega2560)
# set(pro.upload.protocol wiring)
set(pro.upload.speed 115200)

file(GLOB_RECURSE LIB_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/${SDK_NAME}/lib/*.h)
file(GLOB_RECURSE SRC_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/${SDK_NAME}/src/*.h)

#set(LIB_HRDS ${CMAKE_CURRENT_SOURCE_DIR}/${SDK_NAME}/lib)
#set(SRC_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/${SDK_NAME}/src)

generate_arduino_firmware(${CMAKE_PROJECT_NAME} LIBS "${BOARD}_${SDK_NAME}" HDRS ${LIB_HDRS} ${SRC_HDRS})
