#pragma once

#include "ecs/njSystem.h"

namespace njin::ecs {

    class njAnimationSystem : public njSystem {
        public:
        explicit njAnimationSystem(float& delta_time);

        void update(const njEntityManager& entity_manager) override;

        private:
        float& delta_time_;
    };

}  // namespace njin::ecs
