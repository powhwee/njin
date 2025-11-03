#include "ecs/njObjectArchetype.h"

namespace njin::ecs {
    njObjectArchetype::njObjectArchetype(const njObjectArchetypeCreateInfo&
                                         info) :
        info_{ info } {}

    EntityId njObjectArchetype::make_archetype(ecs::njEntityManager&
                                               entity_manager) const {
        EntityId id{ entity_manager.add_entity(info_.name) };
        entity_manager.add_component(id, info_.transform);
        entity_manager.add_component(id, info_.mesh_component);

        return id;
    }
}  // namespace njin::ecs