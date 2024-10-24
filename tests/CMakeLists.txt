set(BINARY tests)

set(SOURCES
    bb.cpp
    graph.cpp
    dfo_rpo.cpp
    domTree.cpp
    instructions.cpp
    main.cpp
)

add_executable(${BINARY} ${SOURCES})

add_test(NAME ${BINARY} COMMAND ${BINARY})

target_link_libraries(${BINARY} PUBLIC domTree irGen gtest)