#pragma once
#include <vector>

#include "ecs/EngineTypes.h"
#include "ecs/njEntityManager.h"

namespace njin::ecs {
    class njArchetype {
        public:
        /**
        * Create a new entity out of this archetype
        * @param entity_manager Entity manager to create this archetype in
        * @return Entity ID of the newly created entity
        */
        virtual EntityId
        make_archetype(ecs::njEntityManager& entity_manager) const = 0;

        virtual ~njArchetype() = default;
    };
}  // namespace njin::ecs