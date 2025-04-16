#include <catch2/catch_test_macros.hpp>
#include "math/njVec4.h"

#include <iostream>

using namespace njin::math;

TEST_CASE("njVec4", "[njVec4]") {
    SECTION("float construction does not throw") {
        REQUIRE_NOTHROW(njVec4{1.0, 2.0, 3.0, 4.0});
    }

    SECTION("default construction is zero vector") {
        njVec4<float> def{};

        njVec4<float> expected{ 0.f, 0.f, 0.f, 0.f };

        REQUIRE(def == expected);
    }

    SECTION("dot product") {
        const njVec4<float> v1{ 1.0, 2.0, 3.0, 4.0 };
        const njVec4<float> v2{ 1.0, 2.0, 3.0, 4.0 };

        constexpr float result{ 30.f };

        REQUIRE(dot(v1, v2) == result);

        const njVec4<float> v3{ -1.0, -2.0, -3.0, -4.0 };
        const njVec4<float> v4{ -1.0, -2.0, -3.0, -4.0 };

        REQUIRE(dot(v3, v4) == result);
    }

    SECTION("float vec4 is 16-byte aligned") {
        njVec4<float> vec{};
        size_t size{ sizeof(vec) };

        REQUIRE(size == 16);
    }
}
