cmake_minimum_required(VERSION 3.28)
project(led-audergon C)
include(../../nanopi.cmake)

include_directories(include)
file(GLOB_RECURSE SOURCES "src/*.c")
add_executable(led ${SOURCES})