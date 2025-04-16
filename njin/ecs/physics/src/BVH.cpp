#include "physics/BVH.h"

#include <vector>

namespace njin::ecs::physics {
    BVH::BVH(const std::vector<Primitive>& primitives) {
        if (!primitives.empty()) {
            // recursive construction of bvh
            root_ = std::make_unique<BVHNode>(primitives, entity_to_node_);
        }
    }

    const BVHNode* BVH::get_root() const {
        return root_.get();
    }

    std::vector<EntityId> BVH::get_entities() const {
        return root_->get_entities();
    }

    std::vector<EntityId> BVH::get_overlaps(EntityId entity) const {
        // we know that the entity (the BVHNode that represents it)
        // always overlaps the root node by definition, since
        // the root node contains all bounding boxes by definition,
        // so we can skip checking the root node

        const BVHNode* entity_node{ entity_to_node_.at(entity) };
        std::vector<const BVHNode*> overlapping_leaves{
            root_->get_overlapping_leaves(*entity_node)
        };

        std::vector<EntityId> result{};
        for (const BVHNode* leaf_node : overlapping_leaves) {
            EntityId this_entity{ leaf_node->get_entities().at(0) };
            if (this_entity != entity) {
                result.push_back(leaf_node->get_entities().at(0));
            }
        }

        return result;
    }

    BoundingBox BVH::get_bounding_box(EntityId entity) const {
        const BVHNode* node{ entity_to_node_.at(entity) };
        return node->get_bounding_box();
    }

    OverlappingPairs get_overlapping_pairs(const BVH& bvh) {
        // get all overlapping entity pairs
        std::vector<EntityId> entities{ bvh.get_entities() };
        OverlappingPairs overlapping{};
        for (EntityId this_entity : entities) {
            std::vector<EntityId> overlapping_entities{
                bvh.get_overlaps(this_entity)
            };
            for (EntityId other_entity : overlapping_entities) {
                // Let (A, B) mean that entity A overlaps entity B
                // Then (B, A) is true as well -> (A, B) is the same as (B, A)
                // We avoid inserting such duplicates
                if (!overlapping.contains({ other_entity, this_entity })) {
                    overlapping.insert({ this_entity, other_entity });
                }
            }
        }

        return overlapping;
    }

}  // namespace njin::ecs::physics
