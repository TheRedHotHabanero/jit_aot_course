set(SOURCES
    bb.cpp
    graph.cpp
    instruction.cpp
    irGen.cpp
    )

add_library(irGen STATIC ${SOURCES})

target_sources(irGen PUBLIC
        bb.h
        graph.h
        instruction.h
        irGen.h
        helperBuilderFunctions.h
        )

target_include_directories(irGen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


set(IRGEN_BINARY_DIR ${CMAKE_BINARY_DIR}/irGen)
set(IRGEN_SOURCE_DIR ${CMAKE_SOURCE_DIR}/irGen)

target_link_libraries(irGen)