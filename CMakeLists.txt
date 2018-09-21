cmake_minimum_required(VERSION 2.8.4)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)

set(PROJECT_NAME Poweroid_SDK_10)

set(DEVICE_NAME timer_button_1x1)
#set(DEVICE_NAME fan_3x1_dht)

project(${DEVICE_NAME})

set(${CMAKE_PROJECT_NAME}_SKETCH ${DEVICE_NAME}/poweroid_${DEVICE_NAME}.ino)

link_directories(Poweroid_SDK_10)

#### Uncomment below additional settings as needed.
set(${CMAKE_PROJECT_NAME}_BOARD pro)
set(${CMAKE_PROJECT_NAME}_PORT COM4)
# set(pro.build.mcu atmega2560)
# set(pro.upload.protocol wiring)
# set(pro.upload.speed 115200)

generate_arduino_firmware(${CMAKE_PROJECT_NAME})
