#pragma once
#include "ecs/Components.h"
#include "ecs/njEntityManager.h"
#include "ecs/njSystem.h"

namespace njin::ecs {
    /**
     * For entites in a hierarchical relationship with other entities
     * (has a parent component), resolves their transform components
     */
    class njSceneGraphSystem final : public njSystem {
        public:
        njSceneGraphSystem();
        void update(const ecs::njEntityManager& entity_manager) override;


        private:
        std::vector<View<njTransformComponent>> no_parent_views_;
        std::vector<View<njParentComponent, njTransformComponent>>
        has_parent_views_;

        /**
         * Update all global transforms to match local transforms
         * This makes it include entities that have had their parent component
         * removed in the last tick.
         * If this is the case, they need their global transform set
         * back to match their local transform
         */
        void update_all() const;

        /**
         * Update the global transform of entities with parents
         */
        void update_has_parents() const;

        const ecs::njEntityManager* entity_manager_{ nullptr };
    };
}  // namespace njin::ecs