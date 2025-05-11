set(BINARY tests)

set(SOURCES
    testBase.cpp
    bb.cpp
    graph.cpp
    dfo_rpo.cpp
    domTree.cpp
    instructions.cpp
    loopChecker.cpp
    peepholes.cpp
    inline.cpp
    main.cpp
)

add_executable(${BINARY} ${SOURCES})

add_test(NAME ${BINARY} COMMAND ${BINARY})

target_link_libraries(${BINARY} PUBLIC domTree irGen gtest)
target_link_libraries(${BINARY} PUBLIC domTree irGen optimizations gtest)