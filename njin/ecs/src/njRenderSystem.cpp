#include "ecs/njRenderSystem.h"

#include <numbers>
#include <cmath>

#include "core/Types.h"
#include "ecs/Components.h"

namespace njin::ecs {
    namespace {
        using namespace njin;

        /**
     * Calculate the current view matrix
     * @param transform Transform of the camera
     * @param camera Camera settings
     * @return View matrix
     */
        math::njMat4f
        calculate_view_matrix(const ecs::njTransformComponent& transform,
                              const ecs::njCameraComponent& camera) {
            using namespace math;
            // location of camera in global space is the last column
            // of the global transform
            // since the camera is always exactly at the origin in local space
            const njVec4f camera_coordinates{ transform.transform.col(3) };
            const njVec3f camera_translation_vec{ camera_coordinates.x,
                                                  camera_coordinates.y,
                                                  camera_coordinates.z };

            // from the target to the camera position
            const njVec3f forward{ normalize(camera_translation_vec -
                                             camera.look_at) };
            const njVec3f right{ normalize(cross(-forward, camera.up)) };
            const njVec3f true_up{ cross(forward, right) };
            // new basis matrix (with view pointing down -z)
            const njVec4f row_0{ right.x, right.y, right.z, 0 };
            const njVec4f row_1{ true_up.x, true_up.y, true_up.z, 0 };
            const njVec4f row_2{ forward.x, forward.y, forward.z, 0 };
            const njVec4f row_3{ 0, 0, 0, 1 };
            const njMat4f camera_basis{ row_0, row_1, row_2, row_3 };

            // first translate everything so the camera is at the origin
            njMat4f camera_translation{ njMat4Type::Translation,
                                        camera_translation_vec };
            math::njMat4f view{ math::inverse(camera_translation) *
                                njMat4f::Identity() };

            // then change the basis so everything is relative to the camera's basis
            view = camera_basis * view;

            return view;
        }

        /**
     * Calculate the projection matrix
     * @param camera Camera settings
     * @return Projection matrix
     */
        math::njMat4f
        calculate_projection_matrix(const ecs::njCameraComponent& camera) {
            if (camera.type == njCameraType::Perspective) {
                auto settings{
                    std::get<PerspectiveCameraSettings>(camera.settings)
                };
                // convert to radians
                const float angle{ settings.horizontal_fov *
                                   std::numbers::pi_v<float> / 180.f };

                // right, left
                const float r{ std::tanf(angle / 2.f) * settings.near };
                const float l{ -r };

                // top, bottom
                const float t{ r / camera.aspect };
                const float b{ -t };

                // near, far aliases
                const auto& n{ settings.near };
                const auto& f{ settings.far };

                return { { (2 * n) / (r - l), 0, (2 * r) / (r - l) - 1, 0 },
                         { 0, (2 * n) / (b - t), (2 * b) / (b - t) - 1, 0 },
                         { 0, 0, f / (n - f), (n * f) / (n - f) },
                         { 0, 0, -1, 0 } };
            } else if (camera.type == njCameraType::Orthographic) {
                auto settings{
                    std::get<OrthographicCameraSettings>(camera.settings)
                };
                const float r{ camera.aspect * settings.scale };
                const float l{ -r };
                const float t{ settings.scale };
                const float b{ -t };

                // near, far aliases
                const auto& n{ settings.near };
                const auto& f{ settings.far };

                return { { 2 / (r - l), 0, 0, (r + l) / (l - r) },
                         { 0, 2 / (b - t), 0, (b + t) / (t - b) },
                         { 0, 0, 1 / (n - f), n / (n - f) },
                         { 0, 0, 0, 1 } };
            }
            return math::njMat4f::Identity(); // Fallback for return warning
        }

        /**
         * Calculate the model matrix (global transform) of a given entity,
         * where the entity has a parent component with a transform
         * @param entity_manager Entity manager
         * @param id Entity to calculate the model transform for
         * @return Model matrix
         */
        math::njMat4f
        calculate_model_matrix(const njEntityManager& entity_manager,
                               EntityId id) {
            // NOTE: this traverses the entire hierarchy for each entity
            // possible future improvement would be to do some caching
            return math::njMat4f::Identity(); // Placeholder return
        }

        /**
         * Create a list of vertices, that, when interpreted as a line list
         * (by a renderer) will draw the wireframe of an AABB.
         * @param centroid Centroid of the AABB, in local space
         * @param x_width
         * @param y_width
         * @param z_width
         * @return
         */
        std::vector<core::njVertex>
        make_wireframe(const math::njVec3f& centroid,
                       float x_width,
                       float y_width,
                       float z_width) {
            float half_x{ x_width / 2 };
            float half_y{ y_width / 2 };
            float half_z{ z_width / 2 };
            float min_x{ centroid.x - half_x };
            float max_x{ centroid.x + half_x };
            float min_y{ centroid.y - half_y };
            float max_y{ centroid.y + half_y };
            float min_z{ centroid.z - half_z };
            float max_z{ centroid.z + half_z };

            math::njVec3f v0{ min_x, min_y, min_z };
            math::njVec3f v1{ min_x, min_y, max_z };
            math::njVec3f v2{ max_x, min_y, max_z };
            math::njVec3f v3{ max_x, min_y, min_z };
            math::njVec3f v4{ min_x, max_y, min_z };
            math::njVec3f v5{ min_x, max_y, max_z };
            math::njVec3f v6{ max_x, max_y, max_z };
            math::njVec3f v7{ max_x, max_y, min_z };

            // assuming we are looking down the -z axis at the front face of
            // the AABB
            std::vector<core::njVertex> line_list{
                v0, v1, v1, v5, v5, v4, v4, v0, v1, v2, v2, v6, v6, v5, v5, v1,
                v2, v3, v3, v7, v7, v6, v6, v2, v0, v3, v3, v7, v7, v4, v4, v0,
                v5, v6, v6, v7, v7, v4, v4, v5, v1, v2, v2, v3, v3, v0, v0, v1
            };

            return line_list;
        }
    }  // namespace

    njRenderSystem::njRenderSystem(core::RenderBuffer& buffer,
                               const core::njRegistry<core::njMesh>& mesh_registry,
                               const core::njRegistry<core::njMaterial>& material_registry,
                               const core::njRegistry<core::njTexture>& texture_registry) :
        njSystem{ TickGroup::Four },
        buffer_{ &buffer },
        mesh_registry_{ &mesh_registry },
        material_registry_{ &material_registry },
        texture_registry_{ &texture_registry } {}

    void njRenderSystem::update(const ecs::njEntityManager& entity_manager) {
        // camera
        const auto camera_views{
            entity_manager.get_views<njTransformComponent, njCameraComponent>()
        };

        const auto camera{
            std::get<njCameraComponent*>(camera_views[0].second)
        };
        const auto cam_transform{
            std::get<njTransformComponent*>(camera_views[0].second)
        };

        // update the view and projection matrices
        math::njMat4f view_matrix{ calculate_view_matrix(*cam_transform,
                                                         *camera) };
        buffer_->set_view_matrix(view_matrix);

        math::njMat4f projection_matrix{ calculate_projection_matrix(*camera) };
        buffer_->set_projection_matrix(projection_matrix);

        // meshes
        // make the renderables and write into render buffer
        std::vector<core::Renderable> renderables{};
        auto meshes_with_parents{
            entity_manager.get_views<njMeshComponent, njTransformComponent, core::njMesh>()
        };

        // meshes with no parent entity
        auto meshes_no_parents{
            entity_manager
            .get_views<Include<njMeshComponent, njTransformComponent>,
                       Exclude<njParentComponent>>()
        };
        for (const auto& [entity, view] : meshes_no_parents) {
            auto mesh_component{ std::get<njMeshComponent*>(view) };
            auto transform{ std::get<njTransformComponent*>(view) };
            
            const auto* mesh_data = mesh_registry_->get(mesh_component->mesh);
            if (!mesh_data) {
                std::cout << "[RenderSystem] Could not find mesh: " << mesh_component->mesh << std::endl;
                continue;
            } else {
                 // std::cout << "[RenderSystem] Rendering mesh: " << mesh_component->mesh << " with " << mesh_data->get_primitives().size() << " primitives" << std::endl;
            }

            for (const auto& primitive : mesh_data->get_primitives()) {
                std::string texture_name = mesh_component->texture_override;

                if (texture_name.empty()) {
                    std::string material_name = primitive.get_material_name();
                    
                    if (!material_name.empty()) {
                        const auto* material = material_registry_->get(material_name);
                        
                        if (material && !material->base_color_texture_name.empty()) {
                            texture_name = material->base_color_texture_name;
                        }
                    }
                }

                core::MeshData data{
                    .global_transform = transform->transform,
                    .mesh_name = mesh_component->mesh,
                    .texture_name = texture_name,
                };
                core::Renderable renderable{ .type = RenderType::Mesh, .data = data };
                renderables.push_back(renderable);
            }
        }

        buffer_->replace(renderables);
    }
}  // namespace njin::ecs