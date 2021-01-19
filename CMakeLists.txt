cmake_minimum_required(VERSION 3.0)

project(passman)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories(./include)

set(SOURCES
    ./src/main.cpp
)

add_executable(passman ${SOURCES})

target_link_libraries(passman ${CONAN_LIBS})
