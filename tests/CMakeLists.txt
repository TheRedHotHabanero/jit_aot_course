set(BINARY tests)

set(SOURCES
    bb.cpp
    graph.cpp
    instructions.cpp
    fact1.cpp
    fact.cpp
    main.cpp
)

add_executable(${BINARY} ${SOURCES})

add_test(NAME ${BINARY} COMMAND ${BINARY})

target_link_libraries(${BINARY} PUBLIC irGen gtest)