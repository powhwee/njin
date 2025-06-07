#pragma once
#include <queue>

#include "core/RenderBuffer.h"
#include "core/Types.h"
#include "core/njMesh.h"
#include "core/njRegistry.h"
#include "core/njTexture.h"
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

    /**
     * Render info for a billboard.
     * Each billboard acts as the graphical representation of the mesh.
     * The billboard quad exactly encloses the mesh in screen space, and then
     * the sprite (drawn from the same isometric angle) is mapped onto the quad.
     *
     * Billboard quads are universally 6 vertices. The first 3 vertices form the
     * upper-left triangle of the quad, and the next 3 vertices form the
     * lower-left triangle of the quad. The winding order is counter-clockwise.
     * Each quad is specified in their mesh's object space
     */
    struct BillboardRenderInfo {
        uint32_t billboard_offset;  // start vertex index of billboard

        // index of model matrix for the mesh this billboard is representing
        uint32_t model_index;
        uint32_t texture_index;  // sprite index
    };

    struct ColliderRenderInfo {
        uint32_t collider_offset;  // start vertex index of collider

        // index of world transform for this collider
        uint32_t transform_index;
    };

    /**
     * Render info along with a renderpass-subpass key.
     * This informs the renderer as to which renderpass/subpass this piece
     * of rendering information should go to.
     */
    using RenderKey = std::pair<std::string, std::string>;

    struct KeyedRenderInfo {
        RenderKey key;
        RenderType type{ RenderType::Mesh };
        std::variant<MeshRenderInfo, BillboardRenderInfo, ColliderRenderInfo>
        info;
    };

    /**
     * Render info stripped of its key. This is the general package of info
     * passed to a SubpassModule. The SubpassModule will read from this
     * and figure out how to do its vkCmd* render calls.
     */
    struct RenderInfo {
        RenderType type{ RenderType::Mesh };
        std::variant<MeshRenderInfo, BillboardRenderInfo, ColliderRenderInfo>
        info;
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
                    const core::njRegistry<core::njTexture>& texture_registry,
                    vulkan::RenderResources& render_resources,
                    const core::RenderBuffer& render_buffer);

        std::vector<RenderInfo> get_render_infos(const RenderKey& key) const;

        private:
        const core::RenderBuffer* render_buffer_;
        RenderBuckets render_infos_;
        TextureIndices texture_indices_;
        /**
         * Update data in render resources, and generate the corresponding
         * RenderInfos
        */
        void write_data(const core::njRegistry<core::njMesh>& mesh_registry,
                        const ::njin::core::njRegistry<core::njTexture>&
                        texture_registry,
                        vulkan::RenderResources& render_resources,
                        const math::njMat4f& view_matrix,
                        const math::njMat4f& projection_matrix);

        /**
         * Load all textures in the given texture registry into the texture descriptor set
         * @param resources
         * @param texture_registry Registry containing all textures to be loaded
         */
        void process_textures(vulkan::RenderResources& resources,
                              const core::njRegistry<core::njTexture>&
                              texture_registry);
    };

}  // namespace njin::vulkan