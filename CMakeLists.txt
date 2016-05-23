# CMAkeLists.txt file for mumpi project
cmake_minimum_required(VERSION 2.8.9)

# Options. Turn on with 'cmake -Dvarname=ON'.
option(test "Build all tests." OFF) # makes boolean 'test' available

project(mumpi)
add_subdirectory("${PROJECT_SOURCE_DIR}/deps/mumlib")
if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_CXX_FLAGS "--std=gnu++11 ${CMAKE_CXX_FLAGS}")
    endif ()
  else ()
    set (CMAKE_CXX_STANDARD 11)
endif ()
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# INCLUDES
include_directories(include)
include_directories("${PROJECT_SOURCE_DIR}/deps/mumlib/include")

# SOURCES
#Can manually add the sources using the set command as follows:
#set(SOURCES src/mainapp.cpp src/Student.cpp)
file(GLOB SOURCES "src/*.cpp")

add_executable(mumpi ${SOURCES})

# LINKING
target_link_libraries(mumpi portaudio)
target_link_libraries(mumpi mumlib)

# TESTING

enable_testing()
add_subdirectory("${PROJECT_SOURCE_DIR}/deps/googletest")

file(GLOB TESTS "test/*.cpp")

add_executable(runUnitTests ${TESTS})
target_link_libraries(runUnitTests gtest gtest_main)
add_test(NAME mumpi-test COMMAND runUnitTests)
