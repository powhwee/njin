#include "core/njCamera.h"

#include <numbers>

#include <cmath>

namespace njin::core {

    njCamera::njCamera(const std::string& name,
                       const njCameraSettings& settings) :
        name_{ name },
        far_{ settings.far },
        near_{ settings.near },
        horizontal_fov_{ settings.horizontal_fov },
        aspect_{ static_cast<float>(settings.width) /
                 static_cast<float>(settings.height) } {
        set_projection_matrix();
    }

    void njCamera::look_at(const math::njVec3f& target,
                           const math::njVec3f& up) {
        // we can't exactly "look at" something if we aren't even in the world
        if (!node_) {
            return;
        }

        using namespace math;
        // location of camera in global space is the last column
        // of the global transform
        // since the camera is always exactly at the origin in local space
        const njVec4f camera_coordinates{
            node_->get_global_transform().col(3)
        };
        const njVec3f camera_translation_vec{ camera_coordinates.x,
                                              camera_coordinates.y,
                                              camera_coordinates.z };

        // from the target to the camera position
        const njVec3f forward{ normalize(camera_translation_vec - target) };
        const njVec3f right{ normalize(cross(-forward, up)) };
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
        view_ = math::inverse(camera_translation) * njMat4f::Identity();

        // then change the basis so everything is relative to the camera's basis
        view_ = camera_basis * view_;
    }

    void njCamera::set_node(const njNode* node) {
        node_ = node;
    }

    math::njMat4f njCamera::get_view_matrix() const {
        return view_;
    }

    math::njMat4f njCamera::get_projection_matrix() const {
        return projection_;
    }

    void njCamera::set_projection_matrix() {
        // convert to radians
        const float angle{ horizontal_fov_ * std::numbers::pi_v<float> /
                           180.f };
        // right, left
        const float r{ std::atanf(angle / 2.f) * near_ };
        const float l{ -r };

        // top, bottom
        const float t{ r / aspect_ };
        const float b{ -t };

        // near, far aliases
        const auto& n{ near_ };
        const auto& f{ far_ };

        projection_ = { { (2 * n) / (r - l), 0, (2 * r) / (r - l) - 1, 0 },
                        { 0, (2 * n) / (b - t), (2 * b) / (b - t) - 1, 0 },
                        { 0, 0, f / (n - f), (n * f) / (n - f) },
                        { 0, 0, -1, 0 } };
    }
}  // namespace njin::core