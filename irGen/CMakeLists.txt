set(SOURCES
    bb.cpp
    graph.cpp
    singleInstruction.cpp
    irGen.cpp
    )

add_library(irGen STATIC ${SOURCES})

target_sources(irGen PUBLIC
        input.h
        bb.h
        graph.h
        singleInstruction.h
        irGen.h
        helperBuilderFunctions.h
        )

target_include_directories(irGen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


set(IRGEN_BINARY_DIR ${CMAKE_BINARY_DIR}/irGen)
set(IRGEN_SOURCE_DIR ${CMAKE_SOURCE_DIR}/irGen)

target_link_libraries(irGen)