#pragma once
#include <unordered_map>
#include <vector>

#include "math/njVec3.h"
#include "physics/PhysicsTypes.h"

namespace njin::ecs::physics {

    class BVHNode;
    using EntityNodeMap = std::unordered_map<EntityId, BVHNode*>;

    /**
*  A single node in the BVH
*/
    class BVHNode {
        public:
        /**
        * Constructor.
        * @param primitives List of bounding box primitives to be part of this node
        * @param map Map of entity IDs to the leaf BVHNode that contains that entity.
        * This BVHNode will add itself to the map if it is a leaf node.
        * @param type Axes of relevance in this BVH Node
        */
        explicit BVHNode(const std::vector<Primitive>& primitives,
                         EntityNodeMap& map,
                         BoundingBoxType type);

        /**
         * @return True if this BVH node is a leaf (no children)
         */
        bool is_leaf() const;

        /**
         * Checks if the bounding box associated with this BVH node overlaps
         * the bounding box associated with another BVH node
         * @param other Other BVH node to check against
         * @return True if both nodes overlap
         */
        bool does_overlap(const BVHNode& other) const;

        /**
         * Given a node that overlaps this node, calculate which leaves of the
         * BVH rooted at this node the input node overlaps
         * @param node Input node that overlaps this node
         * @return List of overlapping nodes
         */
        std::vector<const BVHNode*>
        get_overlapping_leaves(const BVHNode& node) const;

        const BVHNode* get_left() const;

        const BVHNode* get_right() const;

        std::vector<EntityId> get_entities() const;

        BoundingBox get_bounding_box() const;

        private:
        // the bounding box that encapsulates all its primitives
        BoundingBox box_{};

        // all entities contained in this node
        // this is 1 if this node is a leaf
        std::vector<EntityId> entities_{};

        std::unique_ptr<BVHNode> left_{ nullptr };
        std::unique_ptr<BVHNode> right_{ nullptr };
        BoundingBoxType type_;
    };

}  // namespace njin::ecs::physics