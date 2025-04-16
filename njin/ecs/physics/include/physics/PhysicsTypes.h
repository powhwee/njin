#pragma once

namespace njin::ecs::physics {
    using EntityId = uint32_t;

    /**
     * AABB bounding box
     */
    struct BoundingBox {
        static BoundingBox make(const math::njVec3f& centroid,
                                float x_width,
                                float y_width,
                                float z_width) {
            return { .centroid = centroid,
                     .min_x = centroid.x - x_width,
                     .max_x = centroid.x + x_width,
                     .min_y = centroid.y - y_width,
                     .max_y = centroid.y + y_width,
                     .min_z = centroid.z - z_width,
                     .max_z = centroid.z + z_width };
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
         * @return True if both bounding boxes overlap
         */
        bool does_overlap(const BoundingBox& other) const {
            if (max_x < other.min_x || other.max_x < min_x)
                return false;
            if (max_y < other.min_y || other.max_y < min_y)
                return false;
            if (max_z < other.min_z || other.max_z < min_z)
                return false;
            return true;
        }
    };

    // Bounding box coordinates are in world space
    using Primitive = std::pair<EntityId, BoundingBox>;
}  // namespace njin::ecs::physics