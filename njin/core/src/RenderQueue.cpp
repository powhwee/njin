#include "core/RenderQueue.h"

namespace {
    using namespace njin;

    /**
     * Update a descriptor set containing a binding for model matrices with
     * new model matrices
     * @param set Descriptor set
     * @param binding Name of binding for model matrices
     * @param models New model matrices
     */
    void update_model_matrices(njin::vulkan::DescriptorSet* set,
                               const std::string& binding,
                               const std::vector<math::njMat4f>& models) {
        set->update_descriptor_data(binding, models);
    }

    /**
     * Update a descriptor set containing a binding for vp matrices
     * new vp matrices
     * @param set Descriptor set
     * @param binding Name of binding for vp mtarices
     * @param view The new view matrix
     * @param projection The new projection matrix
     */
    void update_vp_matrices(njin::vulkan::DescriptorSet* set,
                            const std::string& binding,
                            const math::njMat4f& view,
                            const math::njMat4f& projection) {
        std::vector<math::njMat4f> vp_matrices{ { view, projection } };
        set->update_descriptor_data(binding, vp_matrices);
    }

}  // namespace

namespace njin::core {
    RenderQueue::RenderQueue(const MeshIndices& mesh_indices,
                             const TextureIndices& texture_indices,
                             const RenderBuffer& render_buffer) :
        render_buffer_{ &render_buffer },
        texture_indices_{ texture_indices },
        mesh_indices_{ mesh_indices } {}

    void RenderQueue::serialize(std::vector<vulkan::DescriptorSet*> sets) {
        const std::vector<Renderable> renderables{
            render_buffer_->get_renderables()
        };
        uint32_t counter{ 0 };
        std::vector<math::njMat4f> models{};
        for (const auto& renderable : renderables) {
            models.push_back(renderable.global_transform);

            if (renderable.type == RenderableType::Wireframe) {
                continue;
            }

            // set up render info
            auto [offset, size]{ mesh_indices_.at(renderable.mesh_name) };
            const uint32_t mesh_offset{ offset };
            const uint32_t mesh_size{ size };
            const uint32_t texture_index{
                texture_indices_.at(renderable.texture_name)
            };
            const uint32_t model_index{ counter };
            RenderInfo info{ .model_index = model_index,
                             .mesh_offset = mesh_offset,
                             .vertex_count = mesh_size,
                             .texture_index = texture_index };
            queue_.push(info);

            ++counter;
        }

        // this is hardcoded and assumes descriptor set 0 contains MVP info
        update_model_matrices(sets.at(0), "model", models);
        update_vp_matrices(sets.at(0),
                           "vp",
                           render_buffer_->get_view_matrix(),
                           render_buffer_->get_projection_matrix());
    }

    bool RenderQueue::is_empty() const {
        return queue_.empty();
    }

    RenderInfo RenderQueue::pop() {
        RenderInfo item{ queue_.front() };
        queue_.pop();
        return item;
    }
}  // namespace njin::core