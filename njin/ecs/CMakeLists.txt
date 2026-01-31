set(SOURCES
    src/njEngine.cpp
    src/njSystem.cpp
    src/njInputSystem.cpp
    src/njMovementSystem.cpp
    src/njRenderSystem.cpp
    src/njSceneGraphSystem.cpp
    src/njPhysicsSystem.cpp
    src/njPlayerArchetype.cpp
    src/njCamaraArchetype.cpp
    src/njObjectArchetype.cpp
    physics/src/BVH.cpp
    physics/src/BVHNode.cpp
    src/njSceneLoader.cpp
)

add_subdirectory(physics)

add_library(ecs STATIC ${SOURCES})
target_include_directories(ecs PUBLIC include)
target_link_libraries(ecs PUBLIC SDL3::SDL3 math core physics_system)

set(TEST_SOURCES test/njEntityManager_test.cpp
    test/njMovementSystem_test.cpp
    test/njSceneGraphSystem_test.cpp
    test/njSceneLoader_test.cpp)
add_library(ecs_test OBJECT ${TEST_SOURCES})
target_link_libraries(ecs_test PRIVATE ecs Catch2::Catch2)
