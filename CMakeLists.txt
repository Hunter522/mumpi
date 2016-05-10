# CMAkeLists.txt file for hello-world-cpp project
cmake_minimum_required(VERSION 2.8.9)
project(test-hello-world)

# INCLUDES
include_directories(include)

# SOURCES
#Can manually add the sources using the set command as follows:
#set(SOURCES src/mainapp.cpp src/Student.cpp)
file(GLOB SOURCES "src/*.cpp")

add_executable(test-hello-world ${SOURCES})

# LINKING
target_link_libraries(test-hello-world portaudio)
target_link_libraries(test-hello-world openal)
