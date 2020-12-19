cmake_minimum_required(VERSION 3.5)

project (flameserv)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(flameserv main.cpp)
target_link_libraries(flameserv PRIVATE Threads::Threads)
