#include "ecs/njCameraArchetype.h"

namespace njin::ecs {

    njCameraArchetype::njCameraArchetype(const njCameraArchetypeCreateInfo&
                                         info) :
        info_{ info } {}

    EntityId
    njCameraArchetype::make_archetype(njEntityManager& entity_manager) const {
        EntityId id{ entity_manager.add_entity(info_.name) };
        entity_manager.add_component(id, info_.transform);
        entity_manager.add_component(id, info_.camera);

        return id;
    }
}  // namespace njin::ecs