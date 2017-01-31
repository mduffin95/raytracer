cmake_minimum_required(VERSION 3.6)
project(Computer_Graphics)

set(CMAKE_CXX_STANDARD 11)

INCLUDE_DIRECTORIES(/usr/local/Cellar/sdl/1.2.15/include/SDL)

set(SOURCE_FILES
    Source/SDLauxiliary.h
    Source/raytracer.cpp
    Source/TestModel.h
    Makefile)

add_executable(Computer_Graphics ${SOURCE_FILES})