cmake_minimum_required(VERSION 4.0)
project(LoadBalancer C)

set(CMAKE_C_STANDARD 23)

add_executable(LoadBalancer main.c)
target_link_libraries(LoadBalancer ws2_32)