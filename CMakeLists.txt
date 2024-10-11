################################################################################
#                                                                              #
# File: CMakeLists.txt                                                         #
# CMakeLists to generate and build opencv project for m2 setsis                #
#                                                                              #
# Author: Rin Baudelet, Yorick Geoffre, Florian Klein, Clément Guerin          #
# Date: Oct. 2024                                                              #
#                                                                              #
################################################################################
cmake_minimum_required(VERSION 3.23 FATAL_ERROR)
project(CVforAgriculture CXX VERSION 0.1)

# Project configs
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define target's name
string(TOUPPER "${PROJECT_NAME}" TARGET)

# Dependencies
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Project's sources

set(${TARGET}_SOURCES
    src/main.cpp
)

set(${TARGET}_HEADERS
    
)

# Targets
add_executable(${TARGET} ${${TARGET}_SOURCES} ${${TARGET}_HEADERS})
target_include_directories(${TARGET} ${OpenCV_LIBS})
