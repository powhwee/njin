#pragma once
#include "core/RenderBuffer.h"
#include "ecs/njSystem.h"

namespace njin::ecs {
    /**
     * Renders stuff to the screen
     */
    class njRenderSystem : public njSystem {
        public:
        /**
         * Constructor
         * @param buffer Buffer for this render system to write into each tick
         */
        njRenderSystem(core::RenderBuffer& buffer);
        void update(const ecs::njEntityManager& entity_manager) override;

        private:
        core::RenderBuffer* buffer_;
    };
}  // namespace njin::ecs