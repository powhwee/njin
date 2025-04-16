#include "physics/BVHNode.h"

#include <ranges>

#include <algorithm>

namespace njin::ecs::physics {
    namespace {
        enum class Axis {
            X,
            Y,
            Z
        };

        /**
         * Gather all the minimums of the given primitives along a particular axis
         * @param axis Axis to gather minimums along
         * @param primitives Primitives to process
         * @return List of minimums
         */
        std::vector<float>
        get_mins(Axis axis, const std::vector<BoundingBox>& primitives) {
            std::vector<float> mins{};
            if (axis == Axis::X) {
                for (const auto& primitive : primitives) {
                    mins.push_back(primitive.min_x);
                }
            } else if (axis == Axis::Y) {
                for (const auto& primitive : primitives) {
                    mins.push_back(primitive.min_y);
                }
            } else if (axis == Axis::Z) {
                for (const auto& primitive : primitives) {
                    mins.push_back(primitive.min_z);
                }
            }
            return mins;
        }

        /**
         * Gather all the maximums of the given primitives along a particular axis
         * @param axis Axis to gather maximums along
         * @param primitives Primitives to process
         * @return List of maximums
         */
        std::vector<float>
        get_maxs(Axis axis, const std::vector<BoundingBox>& primitives) {
            std::vector<float> maxs{};
            if (axis == Axis::X) {
                for (const auto& primitive : primitives) {
                    maxs.push_back(primitive.max_x);
                }
            } else if (axis == Axis::Y) {
                for (const auto& primitive : primitives) {
                    maxs.push_back(primitive.max_y);
                }
            } else if (axis == Axis::Z) {
                for (const auto& primitive : primitives) {
                    maxs.push_back(primitive.max_z);
                }
            }
            return maxs;
        }

        /**
        *  Select the partition axis along which to split the primitives. We choose
        *  the axis that has the largest extent (largest gap between smallest
        *  min and largest max)
        *  @param primitives List of primitives to consider when choosing the
        *  partition axis
        */
        Axis choose_partition_axis(const std::vector<Primitive>& primitives) {
            std::vector<BoundingBox> boxes{};
            for (const auto& box : primitives | std::views::values) {
                boxes.push_back(box);
            }
            namespace ranges = std::ranges;
            namespace views = ranges::views;
            // collect the mins and maxes
            std::vector<float> mins_x{ get_mins(Axis::X, boxes) };
            std::vector<float> mins_y{ get_mins(Axis::Y, boxes) };
            std::vector<float> mins_z{ get_mins(Axis::Z, boxes) };

            std::vector<float> maxs_x{ get_maxs(Axis::X, boxes) };
            std::vector<float> maxs_y{ get_maxs(Axis::Y, boxes) };
            std::vector<float> maxs_z{ get_maxs(Axis::Z, boxes) };

            // calculate extents
            float x_extent{ *ranges::max_element(maxs_x) -
                            *ranges::min_element(mins_x) };
            float y_extent{ *ranges::max_element(maxs_y) -
                            *ranges::min_element(mins_y) };
            float z_extent{ *ranges::max_element(maxs_z) -
                            *ranges::min_element(mins_z) };

            if (x_extent > y_extent && x_extent > z_extent) {
                return Axis::X;
            } else if (y_extent > z_extent && y_extent > x_extent) {
                return Axis::Y;
            } else {
                return Axis::Z;
            }
        }

        /**
         * Calculate the minimal bounding box that encloses all given primitives
         * @param primitives Primitives to enclose
         * @return Calculated bounding box
         */
        BoundingBox
        calculate_bounding_box(const std::vector<Primitive>& primitives) {
            std::vector<BoundingBox> boxes{};
            for (const auto& box : primitives | std::views::values) {
                boxes.push_back(box);
            }
            // collect the mins and maxes
            std::vector<float> mins_x{ get_mins(Axis::X, boxes) };
            std::vector<float> mins_y{ get_mins(Axis::Y, boxes) };
            std::vector<float> mins_z{ get_mins(Axis::Z, boxes) };

            std::vector<float> maxs_x{ get_maxs(Axis::X, boxes) };
            std::vector<float> maxs_y{ get_maxs(Axis::Y, boxes) };
            std::vector<float> maxs_z{ get_maxs(Axis::Z, boxes) };

            // calculate overall mins and maxs
            float min_x{ *std::ranges::min_element(mins_x) };
            float max_x{ *std::ranges::max_element(maxs_x) };
            float min_y{ *std::ranges::min_element(mins_y) };
            float max_y{ *std::ranges::max_element(maxs_y) };
            float min_z{ *std::ranges::min_element(mins_z) };
            float max_z{ *std::ranges::max_element(maxs_z) };
            float centroid_x{ (max_x - min_x) / 2 };
            float centroid_y{ (max_y - min_y) / 2 };
            float centroid_z{ (max_z - min_z) / 2 };

            return {
                .centroid = { centroid_x, centroid_y, centroid_z },
                .min_x = min_x,
                .max_x = max_x,
                .min_y = min_y,
                .max_y = max_y,
                .min_z = min_z,
                .max_z = max_z,
            };
        }
    }  // namespace

    BVHNode::BVHNode(const std::vector<Primitive>& primitives,
                     EntityNodeMap& map) {
        Axis partition_axis{ choose_partition_axis(primitives) };

        // sort it along the axis
        auto compare{ [partition_axis](const Primitive& a,
                                       const Primitive& b) -> bool {
            if (partition_axis == Axis::X) {
                return a.second.centroid.x < b.second.centroid.x;

            } else if (partition_axis == Axis::Y) {
                return a.second.centroid.y < b.second.centroid.y;
            } else {
                return a.second.centroid.z < b.second.centroid.z;
            }
        } };

        std::vector<Primitive> prims{ primitives };

        // base case: this node is a leaf
        if (primitives.size() == 1) {
            left_ = nullptr;
            right_ = nullptr;
            entities_ = { primitives.front().first };
            Primitive primitive{ primitives.front() };  // there is only one
            box_ = primitive.second;

            // we are a leaf so we register the entity contained in this node
            map[primitive.first] = this;

        } else {
            namespace ranges = std::ranges;
            namespace views = ranges::views;
            ranges::sort(prims, compare);

            // we partition such that each side has an equal number of primitives
            size_t half{ primitives.size() / 2 };
            std::vector<Primitive> first{ prims.begin(), prims.begin() + half };
            std::vector<Primitive> second{ prims.begin() + half, prims.end() };
            left_ = std::make_unique<BVHNode>(first, map);
            right_ = std::make_unique<BVHNode>(second, map);

            std::vector<EntityId> entities{};
            for (const auto& entity : primitives | std::views::keys) {
                entities.push_back(entity);
            }
            entities_ = entities;
            box_ = calculate_bounding_box(primitives);
        }
    }

    bool BVHNode::is_leaf() const {
        return (!left_ && !right_);
    }

    bool BVHNode::does_overlap(const BVHNode& other) const {
        return box_.does_overlap(other.box_);
    }

    const BVHNode* BVHNode::get_left() const {
        return left_.get();
    }

    const BVHNode* BVHNode::get_right() const {
        return right_.get();
    }

    std::vector<EntityId> BVHNode::get_entities() const {
        return entities_;
    }

    BoundingBox BVHNode::get_bounding_box() const {
        return box_;
    };

    std::vector<const BVHNode*>
    BVHNode::get_overlapping_leaves(const BVHNode& node) const {
        if (is_leaf()) {
            if (does_overlap(node)) {
                return { this };
            } else {
                return {};
            }
        }
        std::vector<const BVHNode*> left{ left_->get_overlapping_leaves(node) };
        std::vector<const BVHNode*> right{
            right_->get_overlapping_leaves(node)
        };

        std::vector<const BVHNode*> result{ left.begin(), left.end() };
        result.insert(result.end(), right.begin(), right.end());

        return result;
    }
}  // namespace njin::ecs::physics