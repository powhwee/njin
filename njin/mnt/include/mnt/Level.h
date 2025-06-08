#pragma once
#include "ecs/njEngine.h"

namespace mnt {
    /**
     * Abstract level object for MikopiNoTabi.
     * Manages the creation and registration of entities.
     */
    class Level {
        public:
        /**
         * Constructor
         * @param engine Engine object used to build the level
         */
        explicit Level(njin::ecs::njEngine& engine);

        virtual ~Level() = default;

        /**
         * Start loading the level
         */
        virtual void load() = 0;

        protected:
        njin::ecs::njEngine* engine_{ nullptr };
    };

}  // namespace mnt