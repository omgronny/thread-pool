cmake_minimum_required(VERSION 3.21)
project(thread_pool)

set(CMAKE_CXX_STANDARD 20)

find_package(HPX REQUIRED)

add_executable(thread_pool
        main.cpp
        thread_pool.cpp
        thread_pool.h
        )


target_link_libraries(thread_pool HPX::hpx HPX::wrap_main HPX::iostreams_component)
