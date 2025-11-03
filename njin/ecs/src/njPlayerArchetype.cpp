#include "ecs/njPlayerArchetype.h"

namespace njin::ecs {

    njPlayerArchetype::njPlayerArchetype(const njPlayerArchetypeCreateInfo&
                                         info) :
        info_{ info } {}

    EntityId njPlayerArchetype::make_archetype(ecs::njEntityManager&
                                               entity_manager) const {
        EntityId id{ entity_manager.add_entity(info_.name) };
        entity_manager.add_component(id, info_.transform);
        entity_manager.add_component(id, info_.input);
        entity_manager.add_component(id, info_.mesh_component);
        entity_manager.add_component(id, info_.intent);
        entity_manager.add_component(id, info_.physics);

        return id;
    };

}  // namespace njin::ecs