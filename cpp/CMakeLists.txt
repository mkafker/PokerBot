cmake_minimum_required(VERSION 3.25)
project(PokerBot)


file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS include/*.h)
add_executable(poker ${SOURCES} ${HEADERS})

target_include_directories(poker PUBLIC include)

set_property(TARGET poker PROPERTY CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
