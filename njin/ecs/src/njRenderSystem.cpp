#include "ecs/njRenderSystem.h"

#include <numbers>
#include <cmath>
#include "core/Types.h"
#include "ecs/Components.h"

namespace njin::ecs {
    namespace {
        using namespace njin;

        math::njMat4f calculate_model_matrix(const njEntityManager& entity_manager, EntityId id) {
            return math::njMat4f::Identity();
        }

        std::vector<core::njVertex> make_wireframe(const math::njVec3f& centroid, float x_width, float y_width, float z_width) {
            float half_x = x_width / 2;
            float half_y = y_width / 2;
            float half_z = z_width / 2;
            float min_x = centroid.x - half_x;
            float max_x = centroid.x + half_x;
            float min_y = centroid.y - half_y;
            float max_y = centroid.y + half_y;
            float min_z = centroid.z - half_z;
            float max_z = centroid.z + half_z;

            math::njVec3f v0{ min_x, min_y, min_z };
            math::njVec3f v1{ min_x, min_y, max_z };
            math::njVec3f v2{ max_x, min_y, max_z };
            math::njVec3f v3{ max_x, min_y, min_z };
            math::njVec3f v4{ min_x, max_y, min_z };
            math::njVec3f v5{ min_x, max_y, max_z };
            math::njVec3f v6{ max_x, max_y, max_z };
            math::njVec3f v7{ max_x, max_y, min_z };

            std::vector<core::njVertex> line_list{
                v0, v1, v1, v5, v5, v4, v4, v0, v1, v2, v2, v6, v6, v5, v5, v1,
                v2, v3, v3, v7, v7, v6, v6, v2, v0, v3, v3, v7, v7, v4, v4, v0,
                v5, v6, v6, v7, v7, v4, v4, v5, v1, v2, v2, v3, v3, v0, v0, v1
            };

            return line_list;
        }

        math::njVec3f transform_point(const math::njMat4f& matrix, const math::njVec3f& point) {
            math::njVec4f transformed_vec = matrix * math::njVec4f(point.x, point.y, point.z, 1.0f);
            return math::njVec3f(transformed_vec.x, transformed_vec.y, transformed_vec.z);
        }

        math::njMat4f calculate_view_matrix(const ecs::njTransformComponent& transform, const ecs::njCameraComponent& camera) {
            using namespace math;
            const njVec4f camera_coordinates{ transform.transform.col(3) };
            const njVec3f camera_translation_vec{ camera_coordinates.x, camera_coordinates.y, camera_coordinates.z };
            const njVec3f forward{ normalize(camera_translation_vec - camera.look_at) };
            const njVec3f right{ normalize(cross(-forward, camera.up)) };
            const njVec3f true_up{ cross(forward, right) };
            const njVec4f row_0{ right.x, right.y, right.z, 0 };
            const njVec4f row_1{ true_up.x, true_up.y, true_up.z, 0 };
            const njVec4f row_2{ forward.x, forward.y, forward.z, 0 };
            const njVec4f row_3{ 0, 0, 0, 1 };
            const njMat4f camera_basis{ row_0, row_1, row_2, row_3 };
            njMat4f camera_translation{ njMat4Type::Translation, camera_translation_vec };
            math::njMat4f view{ math::inverse(camera_translation) * njMat4f::Identity() };
            view = camera_basis * view;
            return view;
        }

        math::njMat4f calculate_projection_matrix(const ecs::njCameraComponent& camera) {
            if (camera.type == njCameraType::Perspective) {
                auto settings{ std::get<PerspectiveCameraSettings>(camera.settings) };
                const float angle{ settings.horizontal_fov * std::numbers::pi_v<float> / 180.f };
                const float r{ std::tanf(angle / 2.f) * settings.near };
                const float l{ -r };
                const float t{ r / camera.aspect };
                const float b{ -t };
                const auto& n{ settings.near };
                const auto& f{ settings.far };
                return { { (2 * n) / (r - l), 0, 0, 0 },
                         { 0, (2 * n) / (b - t), 0, 0 },
                         { (r + l) / (r - l), (t + b) / (t - b), f / (n - f), -1 },
                         { 0, 0, (n * f) / (n - f), 0 } };
            }
            else if (camera.type == njCameraType::Orthographic) {
                auto settings{ std::get<OrthographicCameraSettings>(camera.settings) };
                const float r{ camera.aspect * settings.scale };
                const float l{ -r };
                const float t{ settings.scale };
                const float b{ -t };
                const auto& n{ settings.near };
                const auto& f{ settings.far };
                return { { 2 / (r - l), 0, 0, (r + l) / (l - r) },
                         { 0, 2 / (t - b), 0, (b + t) / (t - b) },
                         { 0, 0, 1 / (n - f), n / (n - f) },
                         { 0, 0, 0, 1 } };
            }
            throw std::runtime_error("Unknown camera type");
        }
    }

    njRenderSystem::njRenderSystem(core::RenderBuffer& buffer, core::njRegistry<core::njMesh>& mesh_registry) :
        njSystem{ TickGroup::Four },
        buffer_{ &buffer },
        mesh_registry_{ &mesh_registry } {}

    void njRenderSystem::update(const ecs::njEntityManager& entity_manager) {
        const auto camera_views = entity_manager.get_views<njTransformComponent, njCameraComponent>();
        if (camera_views.empty()) {
            return;
        }

        auto camera = std::get<njCameraComponent*>(camera_views[0].second);
        auto cam_transform = std::get<njTransformComponent*>(camera_views[0].second);

        std::vector<core::Renderable> renderables{};
        auto meshes_no_parents = entity_manager.get_views<Include<njMeshComponent, njTransformComponent>, Exclude<njParentComponent>>();

        math::njVec3f scene_min_bounds{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
        math::njVec3f scene_max_bounds{ std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };
        bool scene_has_meshes = false;

        for (const auto& [entity, view] : meshes_no_parents) {
            scene_has_meshes = true;
            auto mesh_component = std::get<njMeshComponent*>(view);
            auto transform_component = std::get<njTransformComponent*>(view);

            core::MeshData data{
                .global_transform = transform_component->transform,
                .mesh_name = mesh_component->registry_key,
                .texture_name = mesh_component->texture,
            };
            renderables.push_back({ .type = RenderType::Mesh, .data = data });

            const core::njMesh& mesh = *mesh_registry_->get(mesh_component->registry_key);
            math::njVec3f local_min = mesh.min_bounds;
            math::njVec3f local_max = mesh.max_bounds;

            std::array<math::njVec3f, 8> corners = {
                math::njVec3f(local_min.x, local_min.y, local_min.z),
                math::njVec3f(local_max.x, local_min.y, local_min.z),
                math::njVec3f(local_min.x, local_max.y, local_min.z),
                math::njVec3f(local_min.x, local_min.y, local_max.z),
                math::njVec3f(local_max.x, local_max.y, local_min.z),
                math::njVec3f(local_max.x, local_min.y, local_max.z),
                math::njVec3f(local_min.x, local_max.y, local_max.z),
                math::njVec3f(local_max.x, local_max.y, local_max.z)
            };

            for (const auto& corner : corners) {
                math::njVec3f transformed_corner = transform_point(transform_component->transform, corner);
                scene_min_bounds.x = std::min(scene_min_bounds.x, transformed_corner.x);
                scene_min_bounds.y = std::min(scene_min_bounds.y, transformed_corner.y);
                scene_min_bounds.z = std::min(scene_min_bounds.z, transformed_corner.z);
                scene_max_bounds.x = std::max(scene_max_bounds.x, transformed_corner.x);
                scene_max_bounds.y = std::max(scene_max_bounds.y, transformed_corner.y);
                scene_max_bounds.z = std::max(scene_max_bounds.z, transformed_corner.z);
            }
        }

        // if (scene_has_meshes) {
        //     math::njVec3f scene_center = (scene_min_bounds + scene_max_bounds) * 0.5f;
        //     math::njVec3f scene_extents = scene_max_bounds - scene_min_bounds;
        //     float max_dim = std::max({ scene_extents.x, scene_extents.y, scene_extents.z });

        //     auto perspective_settings = std::get<PerspectiveCameraSettings>(camera->settings);
        //     float fov_rad = perspective_settings.horizontal_fov * (std::numbers::pi_v<float> / 180.0f);
        //     float camera_distance = (max_dim / 2.0f) / std::tan(fov_rad / 2.0f) * 1.5f;

        //     cam_transform->transform = ecs::njTransformComponent::make(scene_center.x, scene_center.y, scene_center.z + camera_distance).transform;
        //     camera->look_at = scene_center;
        // }

        math::njMat4f view_matrix = calculate_view_matrix(*cam_transform, *camera);
        buffer_->set_view_matrix(view_matrix);

        math::njMat4f projection_matrix = calculate_projection_matrix(*camera);
        buffer_->set_projection_matrix(projection_matrix);

        buffer_->replace(renderables);
    }
} // namespace njin::ecs