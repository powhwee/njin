#include "core/njRegistry.h"

#include <catch2/catch_test_macros.hpp>

#include "core/loader.h"
#include "core/njMesh.h"

namespace njin::core {

    TEST_CASE("njRegistry", "[core][njRegistry]") {
        SECTION("mesh registry loading") {
            SECTION("valid json") {
                njRegistry<njMesh> registry{};
                REQUIRE_NOTHROW(load_meshes("njRegistry/one.meshes", registry));

                njMesh* cube{ registry.get("cube") };
                REQUIRE(cube != nullptr);

                njMesh* sphere{ registry.get("sphere") };
                REQUIRE(sphere != nullptr);
            }
            SECTION("throw if .meshes json is invalid") {
                njRegistry<njMesh> registry{};
                REQUIRE_THROWS(load_meshes("njRegistry/two.meshes", registry));
            }
        }
    }
}  // namespace njin::core
