#pragma once
#include "core/RenderBuffer.h"
#include "ecs/njSystem.h"
#include "core/loader.h" // Include for njRegistry

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
        njRenderSystem(core::RenderBuffer& buffer, core::njRegistry<core::njMesh>& mesh_registry);
        void update(const ecs::njEntityManager& entity_manager) override;

        private:
        core::RenderBuffer* buffer_;
        core::njRegistry<core::njMesh>* mesh_registry_;
    };
}  // namespace njin::ecs