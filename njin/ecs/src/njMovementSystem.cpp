
#include "ecs/njMovementSystem.h"

#include <ecs/Components.h>

namespace {
    using namespace njin;

    /**
     * Calculate the direction of movement based on the input information
     * @param input Input component
     * @return Unit vector representing the direction of movement
     */
    math::njVec3f calculate_direction(const ecs::njInputComponent& input) {
        const float w{ input.w ? 1.f : 0.f };
        const float a{ input.a ? 1.f : 0.f };
        const float s{ input.s ? -1.f : 0.f };
        const float d{ input.d ? -1.f : 0.f };

        float net_forward{ w + s };
        float net_left{ a + d };
        math::njVec3f direction{ net_left, 0.f, net_forward };

        if (net_forward == 0.f && net_left == 0.f) {
            return math::njVec3f(0.f, 0.f, 0.f);
        } else {
            return math::normalize(direction);
        }
    }

}  // namespace

namespace njin::ecs {
    njMovementSystem::njMovementSystem() : njSystem{ TickGroup::One } {}

    void njMovementSystem::update(const ecs::njEntityManager& entity_manager) {
        auto views{ entity_manager
                    .get_views<njInputComponent, njMovementIntentComponent>() };
        for (auto& view : views) {
            auto input{ std::get<njInputComponent*>(view.second) };
            auto intent{ std::get<njMovementIntentComponent*>(view.second) };

            math::njVec3f direction{ calculate_direction(*input) };

            // directly override the velocity
            intent->velocity = direction;
        }
    }

    math::njMat4f
    njMovementSystem::move(const math::njMat4f& old,
                           const math::njVec3f& translation) const {
        const math::njMat4f move_transform{ math::njMat4Type::Translation,
                                            translation };

        return move_transform * old;
    }
}  // namespace njin::ecs
