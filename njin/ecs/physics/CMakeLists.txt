set(SOURCES
    src/BVH.cpp
    src/BVHNode.cpp)
add_library(physics_system STATIC ${SOURCES})
target_include_directories(physics_system PUBLIC include)
target_link_libraries(physics_system PUBLIC math)


set(TEST_SOURCES
    test/BVH_test.cpp)
add_library(physics_system_test OBJECT ${TEST_SOURCES})
target_link_libraries(physics_system_test PRIVATE physics_system Catch2::Catch2)