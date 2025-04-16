set(SOURCES src/file.cpp
    src/GLTFAsset.cpp
    src/Attributes.cpp
    src/BufferView.cpp
    src/Buffer.cpp
    src/NodesBuilder.cpp
    src/Accessor.cpp
    src/json.cpp
    src/stb.cpp)

add_library(util STATIC ${SOURCES})
target_include_directories(util PUBLIC include ${Stb_INCLUDE_DIR})
target_link_libraries(util PUBLIC rapidjson math)


set(TEST_SOURCES test/Buffer_test.cpp)
add_library(util_test OBJECT ${TEST_SOURCES})
target_link_libraries(util_test PRIVATE
                      Catch2::Catch2
                      util)