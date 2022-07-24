cmake_minimum_required(VERSION 3.22)
project(simulador_riscv)

set(CMAKE_CXX_STANDARD 14)

add_executable(clion main.cpp riscv.h globals.h)
