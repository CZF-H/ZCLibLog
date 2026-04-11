cmake_minimum_required(VERSION 3.28.3)
project(ZCLibLog)

set(CMAKE_CXX_STANDARD 11)

add_executable(${PROJECT_NAME}_Example main.cpp)

target_link_libraries(${PROJECT_NAME}_Example)
