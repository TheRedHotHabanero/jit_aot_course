set(SOURCES
   constFolding.cpp
   peepholes.cpp
   staticInline.cpp
   checkElimination.cpp
)

add_library(optimizations STATIC ${SOURCES})

target_sources(optimizations PUBLIC
    constFolding.h
    peepholes.h
    staticInline.h
    pass.h
    checkElimination.h
)
include_directories(${CMAKE_SOURCE_DIR}/irGen)
include_directories(${CMAKE_SOURCE_DIR}/domTree)
target_include_directories(optimizations PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})