cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(COMPILERS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(COMPILERS_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(COMPILERS_BINARY_ROOT ${CMAKE_CURRENT_BINARY_DIR})

#-Wconversion
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wundef -Wwrite-strings -Wcast-qual -Wlogical-op")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${COMPILERS_BINARY_ROOT}/bin)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

set(CMAKE_BUILD_TYPE Debug)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR})

include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(irGen)
add_subdirectory(tests)

# --------------------------clang-format--------------------------------------

file(GLOB_RECURSE ALL_SOURCES_FILES *.cpp *.h)

find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format-14 clang-format)

add_custom_target(clang-force-format
    COMMAND ${CLANG_FORMAT_EXECUTABLE}
    -i
    -style=file
    --verbose
    ${ALL_SOURCES_FILES}
)
