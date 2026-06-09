cmake_minimum_required(VERSION 4.0)
project(RedisServer C)

set(CMAKE_C_STANDARD 23)

add_executable(RedisServer main.c
        Tests.h
        Tests.c)
