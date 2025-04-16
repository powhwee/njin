#include "ecs/njSceneGraphSystem.h"

namespace njin::ecs {

    njSceneGraphSystem::njSceneGraphSystem() : njSystem{ TickGroup::Three } {}

    void
    njSceneGraphSystem::update(const ecs::njEntityManager& entity_manager) {
        entity_manager_ = &entity_manager;
        no_parent_views_ = entity_manager.get_views<njTransformComponent>();
        has_parent_views_ =
        entity_manager.get_views<njParentComponent, njTransformComponent>();

        update_all();
        update_has_parents();
    }

    void njSceneGraphSystem::update_all() const {
        for (auto& view : no_parent_views_) {
            const auto transform{
                std::get<njTransformComponent*>(view.second)
            };
            // transform->global_transform = transform->local_transform;
        }
    }

    void njSceneGraphSystem::update_has_parents() const {
        for (auto& view : has_parent_views_) {
            const auto parent{ std::get<njParentComponent*>(view.second) };
            const auto parent_view{
                entity_manager_->get_view<njTransformComponent>(parent->id)
            };
            const auto parent_transform{
                std::get<njTransformComponent*>(parent_view.second)
            };
            auto transform{ std::get<njTransformComponent*>(view.second) };
            // transform->global_transform = parent_transform->global_transform *
            //                               transform->local_transform;
        }
    }

}  // namespace njin::ecs