cmake_minimum_required(VERSION 3.20)
project(PassTheControllerCpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
    src/main.cpp
)

add_executable(app ${SOURCES})
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
