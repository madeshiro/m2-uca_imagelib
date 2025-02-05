################################################################################
#                                                                              #
# File: CMakeLists.txt                                                         #
# CMakeLists to generate and build opencv project for m2 setsis                #
#                                                                              #
# Author: Rin Baudelet, Yorick Geoffre, Florian Klein, Clément Guerin          #
# Date: Oct. 2024                                                              #
#                                                                              #
################################################################################
cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
project(CVforAgriculture LANGUAGES CXX VERSION 0.1)

# Project configs
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define target's name
string(TOUPPER "${PROJECT_NAME}" TARGET)

# Dependencies
find_package(OpenCV REQUIRED)
include_directories(include ${OpenCV_INCLUDE_DIRS})

# Project's sources

set(${TARGET}_SOURCES
    src/main.cpp
    src/LineDetector.cpp
    src/PlantDetector.cpp
    src/ImagePreProcessor.cpp
    src/JetPositionChecker.cpp
    src/ProcessingFactory.cpp
)

set(${TARGET}_HEADERS
    include/ProcessingFactory.hpp
    include/LineDetector.hpp
    include/LaserBehavior.hpp
    include/JetPositionChecker.hpp
    include/ImagePreProcessor.hpp
    include/Plant.hpp
    include/PlantDetector.hpp
    include/Species.hpp
)

# Targets
add_executable(${TARGET} ${${TARGET}_SOURCES} ${${TARGET}_HEADERS})
target_link_libraries(${TARGET} ${OpenCV_LIBS})

