#pragma once
#include <queue>

#include "core/RenderBuffer.h"
#include "core/Types.h"
#include "core/njMesh.h"
#include "core/njRegistry.h"
#include "vulkan/DescriptorSet.h"
#include "vulkan/RenderResources.h"
#include "vulkan/config.h"

namespace njin::vulkan {
    /**
     * Render info types
     */

    /**
     * Render info for a mesh
     */
    struct MeshRenderInfo {
        uint32_t model_index;   // index of model matrix
        uint32_t mesh_offset;   // start vertex index of mesh
        uint32_t vertex_count;  // number of vertices in mesh
    };

    struct WireframeRenderInfo {};

    /**
     * Render info along with a renderpass-subpass key.
     * This informs the renderer as to which renderpass/subpass this piece
     * of rendering information should go to.
     */
    using RenderKey = std::pair<std::string, std::string>;

    struct KeyedRenderInfo {
        RenderKey key;
        RenderType type{ RenderType::Mesh };
        std::variant<MeshRenderInfo, WireframeRenderInfo> info;
    };

    /**
     * Render info stripped of its key. This is the general package of info
     * passed to a SubpassModule. The SubpassModule will read from this
     * and figure out how to do its vkCmd* render calls.
     */
    struct RenderInfo {
        RenderType type{ RenderType::Mesh };
        std::variant<MeshRenderInfo, WireframeRenderInfo> info;
    };

    /**
     * Keeps track of which render infos belong under which renderpass-subpass keys
     */
    class RenderBuckets {
        public:
        RenderBuckets() = default;

        void add(const KeyedRenderInfo& info);

        std::vector<RenderInfo> get(const RenderKey& key) const;

        private:
        struct Hash {
            std::size_t operator()(const RenderKey& key) const {
                std::hash<std::string> string_hasher{};
                return string_hasher(key.first) ^
                       (string_hasher(key.second) << 1);
            }
        };

        std::unordered_map<RenderKey, std::vector<RenderInfo>, Hash>
        render_infos_{};
    };

    /**
* Serialises the contents of a render buffer into a queue
* to be passed to renderer. The renderer will consume queue items one by one,
* rendering each one.
* The contents of each item in the queue is sufficient to let the
* renderer know how / what to render. This includes mesh offsets and indices
* into the vertex buffer, etc.
*/
    class RenderInfos {
        public:
        RenderInfos(const core::njRegistry<core::njMesh>& mesh_registry,
                    vulkan::RenderResources& render_resources,
                    const core::RenderBuffer& render_buffer);

        std::vector<RenderInfo> get_render_infos(const RenderKey& key) const;

        private:
        const core::RenderBuffer* render_buffer_;
        RenderBuckets render_infos_;
        /**
         * Update data in render resources, and generate the corresponding
         * RenderInfos
        */
        void write_data(const core::njRegistry<core::njMesh>& mesh_registry,
                        vulkan::RenderResources& render_resources,
                        const math::njMat4f& view_matrix,
                        const math::njMat4f& projection_matrix);
    };

}  // namespace njin::vulkan