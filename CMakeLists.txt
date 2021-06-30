cmake_minimum_required(VERSION 3.5)

project(ecs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ecs main.cpp TypeList.hpp entity.hpp tuple_of_optionals.hpp)