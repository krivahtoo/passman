cmake_minimum_required(VERSION 3.0)

project(passwordmanager)

#install(TARGETS passwordmanager RUNTIME DESTINATION bin)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories(./include)

set(SOURCES
    ./src/main.cpp
)

add_executable(passwordmanager ${SOURCES})

target_link_libraries(passwordmanager ${CONAN_LIBS})
