#include "core/MeshBuilder.h"

#include <catch2/catch_test_macros.hpp>

namespace njin::core {

    TEST_CASE("MeshBuilder", "[core][MeshBuilder]") {
        SECTION("constructs without throw") {
            std::vector<uint16_t> indices = { 0, 1, 2,  3,  4,  5,  6,  7,
                                              8, 9, 10, 11, 12, 13, 14, 15 };
            REQUIRE_NOTHROW(MeshBuilder(indices));
        }

        SECTION("add_position_attributes does not throw") {
            MeshBuilder builder{ { 1, 2, 3 } };
            std::vector<math::njVec3f> positions_0{};
            std::vector<math::njVec3f> positions_1{ { 1, 2, 3 } };

            REQUIRE_NOTHROW(builder.add_position_attributes(positions_0));
            REQUIRE_NOTHROW(builder.add_position_attributes(positions_1));
        }

        SECTION("add_normal_attributes does not throw") {
            MeshBuilder builder{ { 1, 2, 3 } };
            std::vector<math::njVec3f> normals_0{};
            std::vector<math::njVec3f> normals_1{ { 1, 2, 3 } };

            REQUIRE_NOTHROW(builder.add_normal_attributes(normals_0));
            REQUIRE_NOTHROW(builder.add_normal_attributes(normals_1));
        }

        SECTION("add_tangent_attributes does not throw") {
            MeshBuilder builder{ { 1, 2, 3 } };
            std::vector<math::njVec4f> tangents_0{};
            std::vector<math::njVec4f> tangents_1{ { 1, 2, 3, 4 } };

            REQUIRE_NOTHROW(builder.add_tangent_attributes(tangents_0));
            REQUIRE_NOTHROW(builder.add_tangent_attributes(tangents_1));
        }
    }

}  // namespace njin::core
