#pragma once
#include "ecs/njSystem.h"

namespace njin::ecs {
    class njInputSystem final : public njSystem {
        public:
        /**
        * Termination flag for the engine
        */
        explicit njInputSystem(bool& should_run);
        void update(const ecs::njEntityManager& entity_manager) override;


        private:
        bool* should_run_;
    };
}  // namespace njin::ecs
