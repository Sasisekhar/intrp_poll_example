cmake_minimum_required(VERSION 3.5)

project(GPT_Example)

# Find the Paho MQTT C++ library
find_package(PahoMqttCpp REQUIRED)

add_executable(GPT main/main.cpp)
include_directories(main)

target_link_libraries(GPT PahoMqttCpp::paho-mqttpp3)
