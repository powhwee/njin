#pragma once

#include "Components.h"
#include "ecs/njArchetype.h"

namespace njin::ecs {
    struct njEnemyArchetypeCreateInfo {
        std::string name;
        njTransformComponent transform;
        njMeshComponent mesh;
        nj2DPhysicsComponent physics;
    };

    class njEnemyArchetype final : public njArchetype {
        public:
        explicit njEnemyArchetype(const njEnemyArchetypeCreateInfo& info);
        EntityId make_archetype(njEntityManager& entity_manager) const override;

        private:
        njEnemyArchetypeCreateInfo info_{};
    };
}  // namespace njin::ecs