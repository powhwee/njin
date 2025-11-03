#pragma once
#include <variant>

#include "core/njMesh.h"
#include "ecs/EngineTypes.h"
#include "math/njMat4.h"

namespace njin::ecs {

    struct njInputComponent {
        bool w{ false };
        bool a{ false };
        bool s{ false };
        bool d{ false };

        bool operator==(const njInputComponent& other) const {
            return w == other.w && a == other.a && s == other.s && d == other.d;
        }
    };

    /**
     * The transform of an entity, following a right-handed coordinate
     * system
     */
    struct njTransformComponent {
        static njTransformComponent make(float x, float y, float z, float scale_x = 1.f, float scale_y = 1.f, float scale_z = 1.f) {
            math::njMat4f translation_matrix{ math::njMat4Type::Translation, { x, y, z } };
            math::njMat4f scale_matrix{ math::njMat4Type::Scale, { scale_x, scale_y, scale_z } };
            return { .transform = translation_matrix * scale_matrix };
        }

        // the global transform of an entity =
        // global transform of parent * local transform of entity
        math::njMat4f transform;

        bool operator==(const njTransformComponent& other) const {
            return transform == other.transform;
        }
    };

    /**
     * Scalars that dictate how to transform raw input values
     */
    struct njMovementIntentComponent {
        math::njVec3f velocity{};
        bool velocity_override{ false };
    };

    /**
     * Mesh data
     */
    struct njMeshComponent {
        std::string registry_key;     // key for mesh in registry
        std::string texture;  // name of texture
    };

    /**
     * Identifies an entity as having a parent (i.e., part of a hierarchy)
     */
    struct njParentComponent {
        EntityId id;  // entity id of parent
    };

    /// Camera ///
    enum class njCameraType {
        Perspective,
        Orthographic,
    };

    /**
     * Perspective camera settings
     * @param near Distasnce from camera to near plane
     * @param far Distance from camera to far plane
     * @param horizontal_fov Horizontal fov of camera, in degrees
     * @param aspect Width/height aspect ratio
     */
    struct PerspectiveCameraSettings {
        float near{ 0 };
        float far{ 0 };
        float horizontal_fov{ 0 };
    };

    struct OrthographicCameraSettings {
        float near{};
        float far{};
        float scale{};
    };

    /**
     * Attaches a camera to an entity
     * The camera uses the entity's location
     */
    struct njCameraComponent {
        njCameraType type{ njCameraType::Perspective };
        math::njVec3f up{};  // up vector of camera
        // point in global space the camera is looking at
        math::njVec3f look_at{};
        float aspect{};
        std::variant<PerspectiveCameraSettings, OrthographicCameraSettings>
        settings;
    };

    // A general bounding box. Not a component in itself, but used as
    // members of components
    // Used to define the bounds of rigid bodies,
    // and also for use as geometry for shape casting
    // Note that this collider specifies bounds in local space
    struct njCollider {
        math::njMat4f transform{};

        // full width, centred on the centroid
        float x_width{ 0 };
        float y_width{ 0 };
        float z_width{ 0 };
    };

    enum class RigidBodyType {
        Static,
        Dynamic
    };

    /**
     * Dynamics state
     */
    struct njPhysicsComponent {
        math::njVec3f velocity{};  // do not edit manually
        math::njVec3f force{};     // do not edit manually
        float mass{ 0 };
        njCollider collider{};          // original aabb representation
        njCollider current_collider{};  // current aabb
        RigidBodyType type{ RigidBodyType::Static };
    };

    /**
     * Specifies shape cast requests (for hitboxes?)
     * // TODO
     */
    struct njCollidersComponent {};
}  // namespace njin::ecs
