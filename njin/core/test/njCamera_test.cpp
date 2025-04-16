#include "core/njCamera.h"

#include <catch2/catch_test_macros.hpp>

namespace njin::core {
    TEST_CASE("njCamera", "[core][njCamera]") {
        SECTION("view") {
            math::njMat4f transform{ math::njMat4Type::Translation,
                                     { 1, 1, 1 } };
            njNode node{ "node", transform };
            njCamera::njCameraSettings settings{ .far = -10,
                                                 .near = -1,
                                                 .horizontal_fov = 60 };
            njCamera camera{ "camera", settings };
            camera.set_node(&node);
            camera.look_at({ 0, 0, 0 }, { 0, 1, 0 });
            math::njMat4f view{ camera.get_view_matrix() };

            using namespace std;
            math::njMat4f expected{
                { 1 / sqrtf(2), 0, -1 / sqrtf(2), 0 },
                { -1 / sqrtf(6), 2 / sqrtf(6), -1 / sqrtf(6), 0 },
                { 1 / sqrtf(3), 1 / sqrtf(3), 1 / sqrtf(3), -3 / sqrtf(3) },
                { 0, 0, 0, 1 }
            };

            REQUIRE(view == expected);
        }
        SECTION("projection") {}
    }
}  // namespace njin::core
