#include "core/njMesh.h"

#include <catch2/catch_test_macros.hpp>

#include "core/njPrimitive.h"

namespace njin::core {

    TEST_CASE("njMesh", "[core][njMesh]") {
        SECTION("constructor no throw") {
            std::vector<njPrimitive> primitives{};
            REQUIRE_NOTHROW(njMesh{ primitives });
        }
    }
}  // namespace njin::core
