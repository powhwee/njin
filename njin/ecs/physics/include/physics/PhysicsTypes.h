#pragma once

namespace njin::ecs::physics {
    using EntityId = uint32_t;

    enum class BoundingBoxType : uint8_t {
        XZ,  // 2D
        XYZ  // 3D
    };

    /**
     * AABB bounding box
     */
    struct BoundingBox {
        static BoundingBox make(const math::njVec3f& centroid,
                                float x_width,
                                float y_width,
                                float z_width) {
            return { .centroid = centroid,
                     .min_x = centroid.x - x_width / 2,
                     .max_x = centroid.x + x_width / 2,
                     .min_y = centroid.y - y_width / 2,
                     .max_y = centroid.y + y_width / 2,
                     .min_z = centroid.z - z_width / 2,
                     .max_z = centroid.z + z_width / 2 };
        }

        math::njVec3f centroid{};  // world space coordinates
        float min_x{ 0 };
        float max_x{ 0 };
        float min_y{ 0 };
        float max_y{ 0 };
        float min_z{ 0 };
        float max_z{ 0 };

        /**
         * Checks if two AABBs overlap
         * @param other Other AABB to check against
         * @param type
         * @return True if both bounding boxes overlap
         */
        bool does_overlap(const BoundingBox& other,
                          BoundingBoxType type) const {
            bool result{ true };
            if (type == BoundingBoxType::XZ) {
                if (max_x < other.min_x || other.max_x < min_x)
                    result = false;
                if (max_z < other.min_z || other.max_z < min_z)
                    result = false;
            } else if (max_y < other.min_y || other.max_y < min_y)
                result = false;
            return result;
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const BoundingBox& box) {
        os << "BoundingBox(\n"
           << "  centroid: (" << box.centroid.x << ", " << box.centroid.y
           << ", " << box.centroid.z << ")\n"
           << "  x: [" << box.min_x << ", " << box.max_x << "]\n"
           << "  y: [" << box.min_y << ", " << box.max_y << "]\n"
           << "  z: [" << box.min_z << ", " << box.max_z << "]\n"
           << ")";
        return os;
    }

    // Bounding box coordinates are in world space
    using Primitive = std::pair<EntityId, BoundingBox>;
}  // namespace njin::ecs::physics