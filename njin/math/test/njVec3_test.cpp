#include "math/njVec3.h"

#include <catch2/catch_test_macros.hpp>

using namespace njin::math;

TEST_CASE("njVec3", "[njVec3]") {
    SECTION("constructs without throwing") {
        REQUIRE_NOTHROW(njVec3(0.0f, 0.0f, 0.0f));
    }
    SECTION("cross product") {
        const njVec3f v_0{ 1, 2, 3 };
        const njVec3f v_1{ 4, 5, 6 };

        const njVec3f expected{ -3, 6, -3 };
        REQUIRE(cross(v_0, v_1) == expected);

        REQUIRE(cross(v_1, v_0) == -expected);
    }

    SECTION("normalization") {
        const njVec3f v_0{ 2, 0, 0 };
        const njVec3f expected{ 1, 0, 0 };
        REQUIRE(normalize(v_0) == expected);
        REQUIRE(normalize(v_0 + v_0) == expected);
    }

    SECTION("magnitude calculation") {
        njVec3f v_0{ 1, 2, 2 };
        float expected_0{ 3 };
        REQUIRE(magnitude(v_0) == expected_0);

        njVec3f v_1{ 1, 4, 8 };
        float expected_1{ 9 };
        REQUIRE(magnitude(v_1) == expected_1);
    }
}