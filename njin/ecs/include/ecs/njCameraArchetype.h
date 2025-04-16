#pragma once
#include <string>

#include "ecs/Components.h"
#include "ecs/njArchetype.h"

namespace njin::ecs {
    struct njCameraArchetypeCreateInfo {
        std::string name;
        njTransformComponent transform;
        njCameraComponent camera;
    };

    class njCameraArchetype final : public njArchetype {
        public:
        explicit njCameraArchetype(const njCameraArchetypeCreateInfo& info);
        EntityId make_archetype(njEntityManager& entity_manager) const override;

        private:
        njCameraArchetypeCreateInfo info_;
    };
}  // namespace njin::ecs