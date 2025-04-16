#include "ecs/njEntityManager.h"

#include <ecs/Components.h>

#include "catch2/catch_test_macros.hpp"

namespace njin::ecs {
    template<typename... T>
    using View = View<T...>;

    TEST_CASE("njEntityManager", "[runtime][njEntityManager]") {
        struct Transform {
            float x;
            float y;
            float z;

            bool operator==(const Transform& other) const {
                return x == other.x && y == other.y && z == other.z;
            }
        };

        struct Input {
            float in;
        };

        njEntityManager manager{};
        Transform transform_0{ 1, 2, 3 };
        Transform transform_1{ 4, 5, 6 };

        Input input_0{ 0 };
        Input input_1{ 1 };

        SECTION("adding components of a single type") {
            // add to non-existent entity
            REQUIRE_THROWS(manager.add_component<Transform>(0, transform_0));
            REQUIRE_NOTHROW(manager.add_entity("zero"));
            REQUIRE_NOTHROW(manager.add_component(0, transform_0));
            REQUIRE_NOTHROW(manager.add_entity("one"));
            REQUIRE_NOTHROW(manager.add_component(1, transform_1));

            std::vector<View<Transform>> transforms{
                manager.get_views<Transform>()
            };
            REQUIRE(transforms.size() == 2);
            const Transform* t_0{ std::get<Transform*>(transforms[0].second) };
            const Transform* t_1{ std::get<Transform*>(transforms[1].second) };
            REQUIRE(*t_0 == transform_0);
            REQUIRE(*t_1 == transform_1);
        }

        SECTION("adding multiple component types") {
            REQUIRE_NOTHROW(manager.add_entity("zero"));
            REQUIRE_NOTHROW(manager.add_component(0, transform_0));
            REQUIRE_NOTHROW(manager.add_component(0, input_0));
            REQUIRE_NOTHROW(manager.add_entity("one"));
            REQUIRE_NOTHROW(manager.add_component(1, transform_1));
            REQUIRE_NOTHROW(manager.add_component(1, input_1));

            std::vector<View<Transform>> transforms{
                manager.get_views<Transform>()
            };
            REQUIRE(transforms.size() == 2);
            std::vector t_and_i{ manager.get_views<Transform, Input>() };
            REQUIRE(t_and_i.size() == 2);
            std::vector<View<Input>> inputs{ manager.get_views<Input>() };
            REQUIRE(inputs.size() == 2);
        }

        SECTION("removing components") {
            manager.add_entity("zero");
            manager.add_entity("one");
            manager.add_component(0, transform_0);

            REQUIRE(manager.get_views<Transform>().size() == 1);
            manager.add_component(1, transform_1);

            REQUIRE(manager.get_views<Transform>().size() == 2);

            manager.remove_components<Transform>(0);
            REQUIRE(manager.get_views<Transform>().size() == 1);

            manager.remove_components<Transform>(1);
            REQUIRE(manager.get_views<Transform>().size() == 0);

            // Entity 0: transform, input
            manager.add_component(0, transform_0);
            manager.add_component(0, input_0);
            REQUIRE(manager.get_views<Transform, Input>().size() == 1);

            // Entity 0: transform, input
            // Entity 1: transform
            manager.add_component(1, transform_1);
            REQUIRE(manager.get_views<Transform, Input>().size() == 1);
            REQUIRE(manager.get_views<Transform>().size() == 2);

            // Entity 0: transform, input
            // Entity 1: transform, input
            manager.add_component(1, input_1);
            REQUIRE(manager.get_views<Transform, Input>().size() == 2);
            REQUIRE(manager.get_views<Transform>().size() == 2);
            REQUIRE(manager.get_views<Input>().size() == 2);
        }

        SECTION("removing entities") {
            manager.add_entity("zero");
            manager.add_component<Transform>(0, transform_0);
            manager.add_component<Input>(0, input_0);
            manager.add_entity("one");
            manager.add_component(1, transform_1);
            manager.add_component(1, input_1);

            REQUIRE(manager.get_views<Transform, Input>().size() == 2);
            manager.remove_entity(0);
            REQUIRE(manager.get_views<Transform, Input>().size() == 1);
            REQUIRE(manager.get_views<Transform>().size() == 1);
            REQUIRE(manager.get_views<Input>().size() == 1);

            manager.remove_entity(1);
            REQUIRE(manager.get_views<Transform, Input>().size() == 0);
            REQUIRE(manager.get_views<Transform>().size() == 0);
            REQUIRE(manager.get_views<Input>().size() == 0);
        }

        SECTION("get view of single entity") {
            REQUIRE_NOTHROW(manager.add_entity("zero"));
            manager.add_component(0, transform_0);
            View view{ manager.get_view<Transform>(0) };
            auto transform{ std::get<Transform*>(view.second) };
            REQUIRE(*transform == transform_0);
        }

        SECTION("get_views include/exclude version") {
            manager.add_entity("zero");
            manager.add_component(0, transform_0);

            REQUIRE(manager.get_views<Include<Transform>, Exclude<>>().size() ==
                    1);
            manager.remove_components<Transform>(0);
            REQUIRE(manager.get_views<Transform>().size() == 0);

            manager.add_component(0, transform_0);
            manager.add_component(0, input_0);

            // non-exclusive
            REQUIRE(manager.get_views<Transform>().size() == 1);

            REQUIRE(manager.get_views<Include<Transform>, Exclude<Input>>()
                    .size() == 0);

            manager.add_entity("one");
            manager.add_component(1, transform_1);
            REQUIRE(manager.get_views<Include<Transform>>().size() == 2);
            manager.add_component(1, input_1);
            REQUIRE(manager.get_views<Include<Transform, Input>>().size() == 2);

            // both entities now have both transform and input components
            REQUIRE(manager.get_views<Include<Transform>, Exclude<Input>>()
                    .size() == 0);
            REQUIRE(manager.get_views<Include<Input>, Exclude<Transform>>()
                    .size() == 0);

            // remove input from entity 1
            manager.remove_components<Input>(1);
            REQUIRE(manager.get_views<Include<Transform>>().size() == 2);
            auto transform_only{
                manager.get_views<Include<Transform>, Exclude<Input>>()
            };
            REQUIRE(transform_only.size() == 1);

            // entity 1 is the only entity with transform only
            REQUIRE(transform_only.at(0).first == 1);

            // remove transform from entity 0
            manager.remove_components<Transform>(0);

            // entity 0 is the only entity with input only
            auto input_only{
                manager.get_views<Include<Input>, Exclude<Transform>>()
            };
            REQUIRE(input_only.size() == 1);
            REQUIRE(input_only.at(0).first == 0);
        }

        SECTION("same component in include/exclude") {
            manager.add_entity("zero");
            manager.add_component(0, transform_0);
            manager.add_component(0, input_0);

            REQUIRE(manager.get_views<Include<Transform, Input>>().size() == 1);

            // if component exists in both include and exclude set
            // the actions is to exclude it
            auto include_exclude{
                manager.get_views<Include<Transform, Input>, Exclude<Input>>()
            };
            REQUIRE(include_exclude.size() == 0);
        }
    }
}  // namespace njin::ecs