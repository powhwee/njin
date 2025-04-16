#pragma once
#include <queue>

#include "core/RenderBuffer.h"
#include "core/Types.h"
#include "vulkan/DescriptorSet.h"

namespace njin::core {
    struct RenderInfo {
        uint32_t model_index;   // index of model matrix
        uint32_t mesh_offset;   // start vertex index of mesh
        uint32_t vertex_count;  // number of vertices in mesh
        uint32_t texture_index;
    };

    /**
* Serialises the contents of a render buffer into a queue
* to be passed to renderer. The renderer will consume queue items one by one,
* rendering each one.
* The contents of each item in the queue is sufficient to let the
* renderer know how / what to render. This includes mesh offsets and indices
* into the vertex buffer, etc.
*/
    class RenderQueue {
        public:
        RenderQueue(const MeshIndices& mesh_indices,
                    const TextureIndices& texture_indices,
                    const RenderBuffer& render_buffer);

        /**
         * Serialise the current state of the render buffer into a queue,
         * ready for the renderer to process. Also updates the descriptor states
         * with the render buffer's data
         * @param sets Descriptor sets used in the current pipeline
         */
        void serialize(std::vector<vulkan::DescriptorSet*> sets);

        bool is_empty() const;

        RenderInfo pop();

        private:
        const RenderBuffer* render_buffer_;
        std::queue<RenderInfo> queue_{};
        TextureIndices texture_indices_;
        MeshIndices mesh_indices_;

        /**
         * Add a renderable to the queue
         * @param renderable Renderable to add
         */
        void add_renderable(const Renderable& renderable);
    };

}  // namespace njin::core