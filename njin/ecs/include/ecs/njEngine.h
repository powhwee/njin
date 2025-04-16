#pragma once
#include <cstdint>
#include <memory>
#include <unordered_map>

#include "ecs/njArchetype.h"
#include "ecs/njEntityManager.h"
#include "ecs/njSystem.h"
#include "njPlayerArchetype.h"

namespace njin::ecs {
    using EntityId = uint32_t;

    /**
     * Main engine class oversees and runs updates on all systems
     */
    class njEngine {
        public:
        njEngine();

        /** Begin forward to entity manager */
        /**
         * Add a system to the engine
         * @param system System to add
         */
        void add_system(std::unique_ptr<njSystem> system);

        /**
         * Add an entity to the entity manager
         * @param name Name of entity
         * @return EntityId of new entity
         */
        EntityId add_entity(const std::string& name);

        /**
         * Remove an entity and all its components
         * @param entity Entity to remove
         */
        void remove_entity(EntityId entity);

        /**
         * Query for a particular view
         * @tparam Component Component types to query for
         * @return List of views
         */
        template<typename... Component>
        std::vector<View<Component...>> get_view() const {
            return entity_manager_.get_views<Component...>();
        }

        /**
         * Remove component(s) from an entity
         * @tparam Component List of components type(s) to remove
         * @param entity Entity to remove components from
         */
        template<typename... Component>
        void remove_components(EntityId entity) {
            entity_manager_.remove_components(entity);
        }

        /**
         * Add a component to an existing entity
         * @tparam Component Type of component
         * @param entity Entity to add component to
         * @param component Component to add
         */
        template<typename Component>
        void add_component(EntityId entity, Component component) {
            entity_manager_.add_component(entity, component);
        }

        /**
         * Query for a particular view of an entity
         * @tparam Component Component types to query for
         * @return View of the given entity
         */
        template<typename... Component>
        View<Component...> get_view(EntityId entity) const {
            return entity_manager_.get_view<Component...>(entity);
        }

        /** End forward to entity manager */

        /**
        * Create a new entity out of a given archetype
        * @param archetype Archetype to make the entity from
        * @return Entity ID of the newly created entity
        */
        EntityId add_archetype(const njArchetype& archetype);

        /**
         * Updates all systems in tick group order
         */
        void update();

        private:
        // systems belonging to the same tick group
        using TickGroupSystems = std::vector<std::unique_ptr<njSystem>>;
        std::unordered_map<TickGroup, TickGroupSystems>
        tick_group_to_systems_{};

        njEntityManager entity_manager_{};

        /**
         * Updates all systems in a tick group
         * @param group Tick group of systems to update
         */
        void update_tick_group(TickGroup group);
    };
}  // namespace njin::ecs