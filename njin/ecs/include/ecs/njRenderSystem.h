#pragma once
#include "core/RenderBuffer.h"
#include "ecs/njSystem.h"
#include "core/njRegistry.h"
#include "core/njMaterial.h"
#include "core/njMaterial.h"
#include "core/njTexture.h"
#include "core/njMesh.h"

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
        njRenderSystem(core::RenderBuffer& buffer,
                       const core::njRegistry<core::njMesh>& mesh_registry,
                       const core::njRegistry<core::njMaterial>& material_registry,
                       const core::njRegistry<core::njTexture>& texture_registry);
        void update(const ecs::njEntityManager& entity_manager) override;

        private:
        core::RenderBuffer* buffer_;
        const core::njRegistry<core::njMesh>* mesh_registry_;
        const core::njRegistry<core::njMaterial>* material_registry_;
        const core::njRegistry<core::njTexture>* texture_registry_;
    };
}  // namespace njin::ecs