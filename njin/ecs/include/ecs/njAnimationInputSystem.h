#pragma once

#include "ecs/njSystem.h"

namespace njin::ecs {

    class njAnimationInputSystem : public njSystem {
        public:
        njAnimationInputSystem() : njSystem(TickGroup::Zero) {}

        void update(const njEntityManager& entity_manager) override;
    };

}  // namespace njin::ecs
