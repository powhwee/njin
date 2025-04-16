#include "core/njSceneReader.h"

#include <catch2/catch_test_macros.hpp>
#include <core/loader.h>

namespace njin::core {

    TEST_CASE("njSceneReader", "[core][njSceneReader]") {
        SECTION("valid scene") {
            njRegistry<njCamera> camera_registry{};
            load_cameras("njSceneReader/valid.cameras", camera_registry);

            njRegistry<njMesh> mesh_registry{};
            load_meshes("njSceneReader/valid.meshes", mesh_registry);

            REQUIRE_NOTHROW(njSceneReader{ "njSceneReader/valid.scene",
                                           camera_registry,
                                           mesh_registry });
        }
        SECTION("invalid scene") {
            njRegistry<njCamera> camera_registry{};
            load_cameras("njSceneReader/valid.cameras", camera_registry);

            njRegistry<njMesh> mesh_registry{};
            load_meshes("njSceneReader/valid.meshes", mesh_registry);
            // .scene file has 0 cameras
            REQUIRE_THROWS(njSceneReader{ "njSceneReader/invalid.scene",
                                          camera_registry,
                                          mesh_registry });
        }
    }

}  // namespace njin::core
