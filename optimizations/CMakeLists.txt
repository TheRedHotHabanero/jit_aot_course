set(SOURCES
    constFolding.cpp
    peepholes.cpp
    )

add_library(optimizations STATIC ${SOURCES})

target_sources(optimizations PUBLIC
constFolding.h
peepholes.h
        )
include_directories(${CMAKE_SOURCE_DIR}/irGen)
include_directories(${CMAKE_SOURCE_DIR}/domTree)
target_include_directories(optimizations PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


set(OPTIMIZATIONS_BINARY_DIR ${CMAKE_BINARY_DIR}/optimizations)
set(OPTIMIZATIONS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/optimizations)

target_link_libraries(optimizations)