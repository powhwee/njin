#include "core/njRegistry.h"

#include <catch2/catch_test_macros.hpp>

#include "core/loader.h"
#include "core/njMesh.h"
#include "core/njMaterial.h"
#include "core/njTexture.h"

namespace njin::core {

    TEST_CASE("njRegistry", "[core][njRegistry]") {
        SECTION("mesh registry loading") {
            SECTION("valid json") {
                njRegistry<njMesh> registry{};
                njRegistry<njMaterial> material_registry{};
                njRegistry<njTexture> texture_registry{};
                REQUIRE_NOTHROW(load_meshes("njRegistry/one.meshes", registry, material_registry, texture_registry));

                njMesh* cube{ registry.get("cube") };
                REQUIRE(cube != nullptr);

                njMesh* sphere{ registry.get("sphere") };
                REQUIRE(sphere != nullptr);
            }
            SECTION("throw if .meshes json is invalid") {
                njRegistry<njMesh> registry{};
                njRegistry<njMaterial> material_registry{};
                njRegistry<njTexture> texture_registry{};
                REQUIRE_THROWS(load_meshes("njRegistry/two.meshes", registry, material_registry, texture_registry));
            }
        }
    }
}  // namespace njin::core
