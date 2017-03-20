cmake_minimum_required(VERSION 2.8.4)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)
set(PROJECT_NAME poweroid_fan_2x1_1x1)
project(${PROJECT_NAME})

set(${CMAKE_PROJECT_NAME}_SKETCH poweroid_fan_2x1_1x1.ino)

#set(SOURCE_FILES
#        poweroid_fan_2x1_1x1.ino
#        poweroid_fan_2x1_1x1_prop.h
#        )
#

link_directories(lib)

#### Uncomment below additional settings as needed.
set(${CMAKE_PROJECT_NAME}_BOARD pro)
set(${CMAKE_PROJECT_NAME}_PORT COM4)
# set(pro.build.mcu atmega2560)
# set(pro.upload.protocol wiring)
# set(pro.upload.speed 115200)

generate_arduino_firmware(${CMAKE_PROJECT_NAME})
