set(SOURCES
    dsu.cpp
    domTree.cpp
    )
add_library(domTree STATIC ${SOURCES})
target_sources(irGen PUBLIC
        domTree.h
        dsu.h
        dfo_rpo.h
        )
include_directories(${CMAKE_SOURCE_DIR}/irGen)
target_include_directories(domTree PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})