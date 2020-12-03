cmake_minimum_required(VERSION 3.17)
project(prak2 C)

set(CMAKE_C_STANDARD 11)
SET(CMAKE_C_FLAGS -pthread)

#add_executable(prak2 main.c queueSemaphore.c queueSemaphore.h)
add_executable(prak2 main.c queueCondVar.c queueCondVar.h)
add_compile_definitions(USE_COND_VARS)