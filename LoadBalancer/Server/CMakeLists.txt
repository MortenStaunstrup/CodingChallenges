cmake_minimum_required(VERSION 4.0)
project(Server C)

set(CMAKE_C_STANDARD 23)

add_executable(Server main.c)
target_link_libraries(Server ws2_32)
