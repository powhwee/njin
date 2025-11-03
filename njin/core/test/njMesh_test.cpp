#include "core/njMesh.h"

#include <catch2/catch_test_macros.hpp>

#include "core/njPrimitive.h"
#include "math/njVec3.h"

namespace njin::core {

    TEST_CASE("njMesh", "[core][njMesh]") {
        SECTION("constructor no throw") {
            std::vector<njPrimitive> primitives{};
            njMeshCreateInfo create_info{
                .name = "test_mesh",
                .primitives = primitives,
                .min_bounds = math::njVec3f{0,0,0},
                .max_bounds = math::njVec3f{0,0,0}
            };
            REQUIRE_NOTHROW(njMesh{ create_info });
        }
    }
}  // namespace njin::core
