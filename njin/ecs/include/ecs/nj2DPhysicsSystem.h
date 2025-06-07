#pragma once
#include <chrono>

#include "math/njMat4.h"
#include "njSystem.h"

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
    class nj2DPhysicsSystem : public njSystem {
        public:
        nj2DPhysicsSystem();

        void update(const ecs::njEntityManager& entity_manager) override;

        private:
        /**
         * Check if the time since the last update has passed the predefined
         * tick interval, which means we should tick again
         * @return True if the physics system should tick
         */
        bool should_update();

        /**
         * Write the transforms for all entities at t_i
         * @param entity_manager Entity manager
         */
        void
        write_current_transforms(const ecs::njEntityManager& entity_manager);

        void
        calculate_new_transforms(const ecs::njEntityManager& entity_manager);
        using TimePoint = std::chrono::steady_clock::time_point;
        using Clock = std::chrono::steady_clock;
        using Duration = std::chrono::steady_clock::duration;
        TimePoint previous_{ Clock::now() };

        std::unordered_map<EntityId, math::njMat4f> transforms_{};
    };
}  // namespace njin::ecs