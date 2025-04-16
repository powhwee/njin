#include "vulkan/RenderInfos.h"

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

namespace njin::vulkan {
    void RenderBuckets::add(const KeyedRenderInfo& info) {
        RenderInfo render_info{ .type = info.type, .info = info.info };
        render_infos_[info.key].push_back(render_info);
    }

    std::vector<RenderInfo> RenderBuckets::get(const RenderKey& key) const {
        return render_infos_.at(key);
    }

    RenderInfos::RenderInfos(const core::njRegistry<core::njMesh>&
                             mesh_registry,
                             vulkan::RenderResources& render_resources,
                             const core::RenderBuffer& render_buffer) :
        render_buffer_{ &render_buffer } {
        write_data(mesh_registry,
                   render_resources,
                   render_buffer.get_view_matrix(),
                   render_buffer.get_projection_matrix());
    }

    std::vector<RenderInfo>
    RenderInfos::get_render_infos(const RenderKey& key) const {
        return render_infos_.get(key);
    }

    void
    RenderInfos::write_data(const core::njRegistry<core::njMesh>& mesh_registry,
                            vulkan::RenderResources& render_resources,
                            const math::njMat4f& view_matrix,
                            const math::njMat4f& projection_matrix) {
        // serialize all the vertices of required meshes into an array
        std::vector<vulkan::MainDrawVertex> vertex_input{};

        // serialize all the corresponding model matrices into an array
        std::vector<vulkan::MainDrawModel> model_matrices{};

        std::vector<math::njMat4f> view_projection{ view_matrix,
                                                    projection_matrix };

        // vertex offset of current mesh into vertex buffer
        uint32_t current_offset{ 0 };
        uint32_t current_model_index{ 0 };
        for (const core::Renderable& renderable :
             render_buffer_->get_renderables()) {
            RenderKey key{ "main", "draw" };
            KeyedRenderInfo info{ .key = key };
            if (renderable.type == RenderType::Mesh) {
                info.type = RenderType::Mesh;
                // add the vertices
                auto data{ std::get<core::MeshData>(renderable.data) };
                const core::njMesh* mesh{ mesh_registry.get(data.mesh_name) };
                std::vector<core::njPrimitive> primitives{
                    mesh->get_primitives()
                };
                for (const core::njPrimitive& primitive : primitives) {
                    std::vector<core::njVertex> vertices{
                        primitive.get_vertices()
                    };
                    for (const core::njVertex& vertex : vertices) {
                        vulkan::MainDrawVertex main_draw_vertex{
                            .x = vertex.position.x,
                            .y = vertex.position.y,
                            .z = vertex.position.z
                        };
                        vertex_input.push_back(main_draw_vertex);
                    }
                }

                // add the model matrices
                vulkan::MainDrawModel main_draw_model{
                    .model = data.global_transform
                };
                model_matrices.push_back(main_draw_model);

                // fill in RenderInfo and add to queue
                MeshRenderInfo mesh_info{
                    .model_index = current_model_index,
                    .mesh_offset = current_offset,
                    .vertex_count = mesh->get_vertex_count()
                };
                info.info = mesh_info;

                render_infos_.add(info);

                // update counters
                current_offset +=
                mesh->get_vertex_count() *
                vulkan::VERTEX_INPUT_INFO_MAIN_DRAW.vertex_size;
                ++current_model_index;
            }

            // write the data to the actual resources
            render_resources.descriptor_sets
            .write_descriptor_data("mvp", "model", model_matrices);

            render_resources.descriptor_sets
            .write_descriptor_data("mvp", "view_projection", view_projection);

            render_resources.vertex_buffers.load_into_buffer("main_draw",
                                                             vertex_input);
        }
    }

}  // namespace njin::vulkan