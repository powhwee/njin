set(SOURCES src/dummy.cpp
)

add_library(runtime STATIC ${SOURCES})
target_include_directories(runtime PUBLIC include)
target_link_libraries(runtime PUBLIC core math)

set(TEST_SOURCES test/njObject_test.cpp
    test/njBuffer_test.cpp)
add_library(runtime_test OBJECT ${TEST_SOURCES})
target_link_libraries(runtime_test PUBLIC runtime Catch2::Catch2)
