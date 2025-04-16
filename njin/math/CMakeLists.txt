set(SOURCES src/dummy.cpp)
add_library(math STATIC ${SOURCES})
target_include_directories(math PUBLIC include)


# test
set(TEST_SOURCES test/njVec4_test.cpp test/njMat4_test.cpp test/njVec3_test.cpp
    test/njVec2_test.cpp)
add_library(math_test OBJECT ${TEST_SOURCES})
target_link_libraries(math_test PUBLIC math Catch2::Catch2)