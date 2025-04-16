#pragma once
#include "core/njNode.h"

namespace njin::core {
    class njCamera {
        public:
        struct njCameraSettings {
            float far{ 0 };
            float near{ 0 };
            float horizontal_fov{ 0 };
            int width{ 0 };
            int height{ 0 };
        };

        /**
         * Constructor
         * @param name Name to give this camera object
         * @param settings Camera settings
         */
        njCamera(const std::string& name, const njCameraSettings& settings);

        /**
         * Orient the camera to look at a specified point in local space
         * @param target Global space coordinate to look at
         * @param up Up vector in global space
         */
        void look_at(const math::njVec3f& target, const math::njVec3f& up);

        /**
         * Set the node this camera references
         * @param node Node this camera references
         */
        void set_node(const njNode* node);

        /**
         * Retrieve the view matrix associated with this camera
         * @return View matrx
         */
        math::njMat4f get_view_matrix() const;

        /**
         * Retrieve the projection matrix associated with this camera
         * @return Projection matrix
         */
        math::njMat4f get_projection_matrix() const;

        private:
        const std::string name_{};
        float far_{};
        float near_{};
        float horizontal_fov_{};
        float aspect_{};
        const njNode* node_{ nullptr };

        math::njMat4f view_{ math::njMat4f::Identity() };
        math::njMat4f projection_{ math::njMat4f::Identity() };

        void set_projection_matrix();
    };
}  // namespace njin::core