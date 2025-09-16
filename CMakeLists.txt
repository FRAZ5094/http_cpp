cmake_minimum_required(VERSION 3.20)
project(http_server VERSION 1.0.0 LANGUAGES CXX)


file(GLOB SOURCES "*.cpp")
add_executable(http_server ${SOURCES})

set_property(TARGET http_server PROPERTY CXX_STANDARD 17)