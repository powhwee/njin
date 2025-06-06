#pragma once
#include <chrono>
#include <unordered_map>

#include <physics/BVH.h>

#include "math/njMat4.h"
#include "njSystem.h"
#include "physics/PhysicsTypes.h"

namespace njin::ecs {
    /**
    * Responsible for directly modifying the transform of entities. This system should
    * be the only system that can directly write to a transform component.
    * Runs at a fixed tick rate.
    *
    * During an update at t_i, the physics system performs 3 main tasks in order.
    * 1. If the entity has a movement intent component, process it and
    * update the physics component (dynamics state) of the entity
    * 2. Write the position of the entity at t_i (which was calculated last tick)
    * to the transform component
    * 3. Use the information in the physics component and the transform component
    * to find the position of the entity at t_(i+1) and store it
    *
    * In step 3, verlet integration is used to find the tentative position of
    * an entity at t_(i+1). Then, a binary BVH is constructed to resolve
    * penetrations based on this tentative position.
    * The BVH is then updated, then collision queries are answered.
    *
    */
    class nj3DPhysicsSystem final : public njSystem {
        public:
        nj3DPhysicsSystem();

        void update(const ecs::njEntityManager& entity_manager) override;

        private:
        /**
         * Check if the time since the last update has passed the predefined
         * tick interval, which means we should tick again
         * @return True if the physics system should tick
         */
        bool should_update();

        using TimePoint = std::chrono::steady_clock::time_point;
        using Clock = std::chrono::steady_clock;
        using Duration = std::chrono::steady_clock::duration;
        TimePoint previous_{ Clock::now() };

        // at the end of tick i of the physics system, this map contains
        // the global transform of an entity at tick (i+1)
        // By the time tick (i+1) comes around, the physics system will
        // write the transform at tick (i+1) to the transform component
        // of the entity. This ensures that the renderer (which updates
        // after the physics engine) will get the latest positional information.

        std::unordered_map<EntityId, math::njMat4f> entity_to_transform_{};

        /**
         * Write the transforms for all entities at t_i
         * @param entity_manager Entity manager
         */
        void
        write_current_transforms(const ecs::njEntityManager& entity_manager);

        /**
         * Calculate the transforms for each entity at t_(i+1)
         * @param entity_manager Entity manager
         * @note These are tentative transforms that have not been
         * depenetrated
         */
        void
        calculate_new_transforms(const ecs::njEntityManager& entity_manager);

        /**
         * Calculate the bounding box primitives for all entities, based
         * on the transforms for the entities at tick (i+1) at the current state
         * @return List of primitives based on the current transforms
         * for tick (i+1)
         */
        std::vector<physics::Primitive>
        calculate_primitives(const njEntityManager& entity_manager) const;

        /**
         * Resolve all penetrating bodies in a given set of primitives
         * @param entity_manager
         * @param primitives Primitives to depenetrate
         * @return Updated BVH with no penetrating objects
         */
        physics::BVH
        depenetrate(const njEntityManager& entity_manager,
                    const std::vector<physics::Primitive>& primitives);
    };

}  // namespace njin::ecs