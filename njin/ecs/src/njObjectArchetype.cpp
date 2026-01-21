#include "ecs/njObjectArchetype.h"

namespace njin::ecs {
    njObjectArchetype::njObjectArchetype(const njObjectArchetypeCreateInfo& info, const core::njRegistry<core::njMesh>& mesh_registry) :
        info_{ info }, mesh_registry_(&mesh_registry) {}

    EntityId njObjectArchetype::make_archetype(ecs::njEntityManager&
                                               entity_manager) const {
        EntityId id{ entity_manager.add_entity(info_.name) };
        entity_manager.add_component(id, info_.transform);
        entity_manager.add_component(id, info_.mesh);

        return id;
    }
}  // namespace njin::ecs