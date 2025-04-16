#pragma once
#include <cstdint>
#include <vector>

#include "math/njMat4.h"

namespace njin::core {
    /**
    * A single node in the scene hierarchy. All nodes in the scene hierarchy
    * are typically set up by njSceneReader instead of built manually
    */
    class njNode {
        public:
        njNode() = default;
        /**
         * Constructor
         * @param name Name to give to this node
         * @param local_transform Local transform of this node
         */
        njNode(const std::string& name, const math::njMat4f& local_transform);

        /**
         * Creates a node with no local transform (identity)
         * @param name
         */
        njNode(const std::string& name);

        math::njMat4f get_local_transform() const;

        math::njMat4f get_global_transform() const;

        std::string get_name() const;

        std::vector<njNode*> get_children() const;

        /**
         * Add a child to this node
         * @param node Pointer to child to add
         */
        void add_child(njNode* node);

        /**
         * Remove a child from this node
         * @param node Pointer to child to remove
         */
        void remove_child(njNode* node);

        /** Set the local transform of this node
         * @param local_transform Local transform
         * @note Also updates global transforms for itself and child nodes accordingly */
        void set_local_transform(const math::njMat4f& local_transform);

        private:
        const std::string name_{};
        math::njMat4f local_transform_{};
        math::njMat4f global_transform_{};
        std::vector<njNode*> children_{};
        njNode* parent_{ nullptr };

        // update the global transform of this node
        void update_global_transform();
    };
}  // namespace njin::core
