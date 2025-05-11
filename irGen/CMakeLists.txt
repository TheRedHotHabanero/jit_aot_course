set(SOURCES
    bb.cpp
    graph.cpp
    singleInstruction.cpp
    graphHelper.cpp
    base.cpp
    instructionsCopy.cpp
    )

add_library(irGen STATIC ${SOURCES})

target_sources(irGen PUBLIC
        input.h
        user.h
        marker.h
        bb.h
        graph.h
        singleInstruction.h
        helperBuilderFunctions.h
        graphHelper.h
        compiler.h
        base.h
        )

target_include_directories(irGen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


set(IRGEN_BINARY_DIR ${CMAKE_BINARY_DIR}/irGen)
set(IRGEN_SOURCE_DIR ${CMAKE_SOURCE_DIR}/irGen)

target_link_libraries(irGen)