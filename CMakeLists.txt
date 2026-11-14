cmake_minimum_required(VERSION 3.20)
project(PassTheControllerCpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
    src/main.cpp
)

add_executable(app ${SOURCES})
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(WIN32)
    target_link_libraries(app PRIVATE ws2_32)
endif()

if(UNIX OR LINUX)
    target_link_libraries(app PRIVATE pthread)
endif()

if(MSVC)
    target_compile_options(app PRIVATE /W4)
else()
    target_compile_options(app PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_test(NAME UnitAndStateTests COMMAND app --test)
add_test(NAME Scenarios COMMAND app --scenario all)

