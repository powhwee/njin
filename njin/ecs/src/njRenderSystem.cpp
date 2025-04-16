#include "ecs/njRenderSystem.h"

#include <numbers>

#include <assert.h>

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
            // convert to radians
            const float angle{ camera.horizontal_fov *
                               std::numbers::pi_v<float> / 180.f };

            const float aspect{ static_cast<float>(camera.width) /
                                static_cast<float>(camera.height) };

            // right, left
            const float r{ std::atanf(angle / 2.f) * camera.near };
            const float l{ -r };

            // top, bottom
            const float t{ r / aspect };
            const float b{ -t };

            // near, far aliases
            const auto& n{ camera.near };
            const auto& f{ camera.far };

            return { { (2 * n) / (r - l), 0, (2 * r) / (r - l) - 1, 0 },
                     { 0, (2 * n) / (b - t), (2 * b) / (b - t) - 1, 0 },
                     { 0, 0, f / (n - f), (n * f) / (n - f) },
                     { 0, 0, -1, 0 } };
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

    njRenderSystem::njRenderSystem(core::RenderBuffer& buffer) :
        njSystem{ TickGroup::Four },
        buffer_{ &buffer } {}

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
            entity_manager.get_views<njMeshComponent, njTransformComponent>()
        };

        // meshes with no parent entity
        auto meshes_no_parents{
            entity_manager
            .get_views<Include<njMeshComponent, njTransformComponent>,
                       Exclude<njParentComponent>>()
        };
        for (const auto& [entity, view] : meshes_no_parents) {
            auto mesh{ std::get<njMeshComponent*>(view) };
            auto transform{ std::get<njTransformComponent*>(view) };
            // global transform = local transform for entities with no parent
            core::Renderable renderable{
                .global_transform = transform->transform,
                .mesh_name = mesh->mesh,
                .texture_name = mesh->texture
            };
            renderables.push_back(renderable);
        }

        // colliders
        auto colliders{
            entity_manager.get_views<njPhysicsComponent, njTransformComponent>()
        };
        for (const auto& [entity, view] : colliders) {
            auto physics_comp{ std::get<njPhysicsComponent*>(view) };
            auto transform_comp{ std::get<njTransformComponent*>(view) };
            njCollider collider{ physics_comp->current_collider };
            math::njMat4f transform{ transform_comp->transform *
                                     collider.transform };
            math::njVec3f centroid{ collider.transform.get_translation_part() };
            std::vector<core::njVertex> line_list{
                make_wireframe(centroid,
                               collider.x_width,
                               collider.y_width,
                               collider.z_width)
            };

            core::Renderable renderable{
                .type = core::RenderableType::Wireframe,
                .global_transform = transform,
                .line_list = line_list
            };

            renderables.push_back(renderable);
        }

        // meshes with parent entities
        auto meshes_parents{ entity_manager.get_views<njMeshComponent,
                                                      njTransformComponent,
                                                      njParentComponent>() };
        for (const auto& [entity, view] : meshes_parents) {
            auto mesh{ std::get<njMeshComponent*>(view) };
            auto transform{ std::get<njTransformComponent*>(view) };
            auto parent{ std::get<njParentComponent*>(view) };

            math::njMat4f current_transform{ transform->transform };

            // TODO: implement
            // there needs to be some way to query the entity manager
            // for whether an entity possesses some component
        }

        buffer_->replace(renderables);
    }
}  // namespace njin::ecs