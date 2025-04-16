#pragma once
#include <cstdint>

#include "njEntityManager.h"

namespace njin::ecs {
    enum class TickGroup : uint8_t {
        Zero = 0,
        One = 1,
        Two = 2,
        Three = 3,
        Four = 4
    };

    class njSystem {
        public:
        virtual ~njSystem() = default;

        njSystem(TickGroup group);
        TickGroup get_tick_group() const;

        virtual void update(const ecs::njEntityManager& entity_manager) = 0;

        protected:
        TickGroup tick_group_;

        private:
    };
}  // namespace njin::ecs