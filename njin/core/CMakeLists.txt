set(SOURCES
    src/njMesh.cpp
    src/njPrimitive.cpp
    src/njVertex.cpp
    src/MeshBuilder.cpp
    src/njSceneReader.cpp
    src/njNode.cpp
    src/njCamera.cpp
    src/njScene.cpp
    src/loader.cpp
    src/RenderBuffer.cpp
    src/njTexture.cpp
    src/RenderQueue.cpp)

add_library(core STATIC ${SOURCES})
target_include_directories(core PUBLIC include)
target_link_libraries(core PUBLIC math rapidjson util Vulkan::Vulkan vulkan)

set(TEST_SOURCES
    test/MeshBuilder_test.cpp
    test/njMesh_test.cpp
    test/njPrimitive_test.cpp
    test/njVertex_test.cpp
    test/njNode_test.cpp
    test/njSceneReader_test.cpp
    test/njRegistry_test.cpp
    test/njScene_test.cpp
    test/njCamera_test.cpp)
add_library(core_test OBJECT ${TEST_SOURCES})
target_link_libraries(core_test PRIVATE core Catch2::Catch2)