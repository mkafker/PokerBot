cmake_minimum_required(VERSION 3.22)
project(PokerBot)


file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS include/*.h)
add_executable(poker ${SOURCES} ${HEADERS})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(poker PRIVATE Threads::Threads)

target_include_directories(poker PUBLIC include)

set_property(TARGET poker PROPERTY CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
