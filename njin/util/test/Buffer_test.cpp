#include "util/Buffer.h"

#include <iostream>

#include <catch2/catch_test_macros.hpp>

using namespace njin::gltf;

TEST_CASE("glTF Buffer", "[gltfbuffer]") {
    SECTION("Buffer copies bytes properly") {
        std::stringstream ss{ "Hello" };
        Buffer buf{ ss, 0, 5 };

        std::vector<std::byte> hell{ buf.get_range(0, 3) };

        REQUIRE(hell.at(0) == static_cast<std::byte>('H'));
        REQUIRE(hell.at(1) == static_cast<std::byte>('e'));
        REQUIRE(hell.at(2) == static_cast<std::byte>('l'));
        REQUIRE(hell.at(3) == static_cast<std::byte>('l'));
    };
}
