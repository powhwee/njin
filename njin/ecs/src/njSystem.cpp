#include "ecs/njSystem.h"

namespace njin::ecs {

    njSystem::njSystem(TickGroup group) : tick_group_{ group } {}

    TickGroup njSystem::get_tick_group() const {
        return tick_group_;
    }
}  // namespace njin::ecs