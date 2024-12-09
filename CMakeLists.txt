cmake_minimum_required(VERSION 3.5)

project(GPT)

# Find the Paho MQTT C++ library
find_package(PahoMqttCpp REQUIRED)

add_executable(GPT main/main.cpp)
target_include_directories(GPT PRIVATE "." "include" $ENV{CADMIUM})

target_link_libraries(GPT PahoMqttCpp::paho-mqttpp3)
