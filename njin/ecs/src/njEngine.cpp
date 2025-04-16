#include "ecs/njEngine.h"

namespace njin::ecs {
    njEngine::njEngine() {}

    void njEngine::add_system(std::unique_ptr<njSystem> system) {
        const TickGroup tick_group{ system->get_tick_group() };

        TickGroupSystems& systems{ tick_group_to_systems_[tick_group] };
        systems.push_back(std::move(system));
    }

    EntityId njEngine::add_entity(const std::string& name) {
        return entity_manager_.add_entity(name);
    }

    void njEngine::remove_entity(EntityId entity) {
        entity_manager_.remove_entity(entity);
    }

    EntityId njEngine::add_archetype(const njArchetype& archetype) {
        EntityId id{ archetype.make_archetype(entity_manager_) };
        return id;
    }

    void njEngine::update() {
        update_tick_group(TickGroup::Zero);
        update_tick_group(TickGroup::One);
        update_tick_group(TickGroup::Two);
        update_tick_group(TickGroup::Three);
        update_tick_group(TickGroup::Four);
    }

    void njEngine::update_tick_group(TickGroup group) {
        if (tick_group_to_systems_.contains(group)) {
            TickGroupSystems& group_systems{ tick_group_to_systems_.at(group) };
            for (auto& system : group_systems) {
                system->update(entity_manager_);
            }
        }
    }
}  // namespace njin::ecs