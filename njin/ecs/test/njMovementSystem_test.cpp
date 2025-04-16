#include <ecs/njMovementSystem.h>

#include "catch2/catch_test_macros.hpp"
#include "ecs/Components.h"
#include "ecs/njEngine.h"

namespace njin::ecs {

    TEST_CASE("njMovementSystem", "[ecs][njMovementSystem]") {
        njEngine engine{};
        engine.add_system(std::make_unique<njMovementSystem>());
        EntityId entity{ engine.add_entity("zero") };
        njInputComponent original_input{ .w = true,
                                         .a = false,
                                         .s = false,
                                         .d = false };
        njMovementIntentComponent movement{};
        engine.add_component(entity, original_input);
        engine.add_component(entity, movement);

        SECTION("add component") {
            auto views{
                engine.get_view<njInputComponent, njMovementIntentComponent>()
            };

            REQUIRE(views.size() == 1);

            auto input{ std::get<njInputComponent*>(views[0].second) };
            REQUIRE(*input == original_input);
        }
    }

}  // namespace njin::ecs