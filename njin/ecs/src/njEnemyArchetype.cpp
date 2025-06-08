#include "ecs/njEnemyArchetype.h"

namespace njin::ecs {
    njEnemyArchetype::njEnemyArchetype(const njEnemyArchetypeCreateInfo& info) :
        info_{ info } {}

    EntityId
    njEnemyArchetype::make_archetype(njEntityManager& entity_manager) const {
        EntityId id{ entity_manager.add_entity(info_.name) };
        entity_manager.add_component(id, info_.transform);
        entity_manager.add_component(id, info_.mesh);
        entity_manager.add_component(id, info_.physics);
        return id;
    }
}  // namespace njin::ecs