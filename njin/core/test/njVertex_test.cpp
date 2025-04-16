#include "core/njVertex.h"

#include <catch2/catch_test_macros.hpp>

namespace njin::core {

    TEST_CASE("njVertex", "[core][njVertex]") {
        SECTION("constructor") {
            SECTION("does not throw") {
                math::njVec3f position{ 1, 2, 3 };
                njVertexCreateInfo info{ .position = position };

                REQUIRE_NOTHROW(njVertex{ info });
            }
        }
    }
}  // namespace njin::core