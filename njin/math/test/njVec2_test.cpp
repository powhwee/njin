#include "math/njVec2.h"

#include <catch2/catch_test_macros.hpp>

using namespace njin::math;

TEST_CASE("njVec2", "[njVec2]") {
    SECTION("default construction") {
        njVec2<float> v{};
        njVec2<float> expected{ 0.f, 0.f };

        REQUIRE(v == expected);
    }

    SECTION("construction does not throw") {
        REQUIRE_NOTHROW(njVec2<float>{});
    }
}