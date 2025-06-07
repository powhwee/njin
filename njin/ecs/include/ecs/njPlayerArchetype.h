#pragma once
#include "Components.h"
#include "ecs/njArchetype.h"

namespace njin::ecs {
    struct njPlayerArchetypeCreateInfo {
        std::string name;
        njTransformComponent transform;
        njInputComponent input;
        njMeshComponent mesh;
        njMovementIntentComponent intent;
        nj2DPhysicsComponent physics;
    };

    class njPlayerArchetype final : public njArchetype {
        public:
        explicit njPlayerArchetype(const njPlayerArchetypeCreateInfo& info);
        EntityId make_archetype(njEntityManager& entity_manager) const override;

        private:
        njPlayerArchetypeCreateInfo info_;
    };

}  // namespace njin::ecs