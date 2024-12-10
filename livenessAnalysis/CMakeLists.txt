set(SOURCES
    livenessAnalyzer.cpp
    liveRanges.cpp
    )
add_library(livenessAnalyzer STATIC ${SOURCES})
target_sources(livenessAnalyzer PUBLIC
        livenessAnalyzer.h
        liveRanges.h
        )
include_directories(${CMAKE_SOURCE_DIR}/irGen)
include_directories(${CMAKE_SOURCE_DIR}/domTree)
target_include_directories(livenessAnalyzer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})