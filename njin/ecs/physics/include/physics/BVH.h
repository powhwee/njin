#pragma once
#include <unordered_map>
#include <vector>

#include "physics/BVHNode.h"

#include <set>

namespace njin::ecs::physics {
    /**
     * A binary Bounding Volume Hierarchy implementation. A "primitive" refers
     * to the unit of a BVH, an abstract representation of real rigid bodies
     * by a single bounding box around a centroid. For now, only AABBs are supported.
     */
    class BVH {
        public:
        explicit BVH(const std::vector<Primitive>& primitives);

        const BVHNode* get_root() const;

        /**
         * Retrieve the list of entities in the BVH
         * @return List of entity ids
         */
        std::vector<EntityId> get_entities() const;

        /**
         * Returns a list of entities that overlap a given entity
         * @param entity Entity to test overlaps for
         * @return List of overlapping entities (excluding this one)
         */
        std::vector<EntityId> get_overlaps(EntityId entity) const;

        /**
         * Get the bounding box of a specified entity
         * @param entity Entity to get the bounding box for
         * @return Bounding box
         */
        BoundingBox get_bounding_box(EntityId entity) const;

        private:
        std::unique_ptr<BVHNode> root_{ nullptr };

        // mapping of an entity to the node that represents it
        EntityNodeMap entity_to_node_{};
    };

    using OverlappingPairs = std::set<std::pair<EntityId, EntityId>>;
    /**
     * Get the set of pairs of overlapping entities within a given BVH
     * @param bvh BVH to check for overlaps
     * @return Set of overlapping pairs
     */
    OverlappingPairs get_overlapping_pairs(const BVH& bvh);

}  // namespace njin::ecs::physics