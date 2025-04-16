#include "ecs/njSceneGraphSystem.h"

#include <catch2/catch_test_macros.hpp>

#include "core/njScene.h"
#include "ecs/njEngine.h"

namespace njin::ecs {

    TEST_CASE("njSceneGraphSystem", "[ecs][njSceneGraphSystem]") {
        njEngine engine{};
        engine.add_system(std::make_unique<njSceneGraphSystem>());
        EntityId child{ engine.add_entity("child") };
        EntityId parent{ engine.add_entity("parent") };
        math::njMat4f child_transform{ { 1.f, 0.f, 0.f, 2.f },
                                       { 0.f, 1.f, 0.f, 2.f },
                                       { 0.f, 0.f, 1.f, 2.f },
                                       { 0.f, 0.f, 0.f, 1.f } };
        njTransformComponent child_comp{ .transform = child_transform };

        math::njMat4f parent_transform{ { 1.f, 0.f, 0.f, 3.f },
                                        { 0.f, 1.f, 0.f, 3.f },
                                        { 0.f, 0.f, 1.f, 3.f },
                                        { 0.f, 0.f, 0.f, 1.f } };

        njTransformComponent parent_comp{ .transform = parent_transform };

        SECTION("transform propagation") {
            engine.add_component(child, child_comp);
            engine.add_component(parent, parent_comp);

            // check that the child's transforms loaded in properly
            auto old_child_view{ engine.get_view<njTransformComponent>(child) };
            REQUIRE(std::get<njTransformComponent*>(old_child_view.second)
                    ->transform == child_transform);

            // check that the parent's transforms loaded in properly
            auto old_parent_view{
                engine.get_view<njTransformComponent>(parent)
            };
            REQUIRE(std::get<njTransformComponent*>(old_parent_view.second)
                    ->transform == parent_transform);

            // parent the child to the parent
            njParentComponent p{ parent };
            engine.add_component(child, p);

            engine.update();

            // TODO: add test for transform propagation
        }
    };

}  // namespace njin::ecs
