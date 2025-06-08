#pragma once
#include "mnt/Level.h"

namespace mnt {
    class TownLevel : public Level {
        public:
        TownLevel(njin::ecs::njEngine& engine);

        void load() override;
    };

}  // namespace mnt