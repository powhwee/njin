#include "core/njNode.h"

namespace njin::core {
    njNode::njNode(const std::string& name,
                   const math::njMat4f& local_transform) :
        name_{ name },
        local_transform_{ local_transform },
        global_transform_{ local_transform } {}

    njNode::njNode(const std::string& name) : name_{ name } {}

    math::njMat4f njNode::get_local_transform() const {
        return local_transform_;
    }

    math::njMat4f njNode::get_global_transform() const {
        return global_transform_;
    }

    std::string njNode::get_name() const {
        return name_;
    }

    std::vector<njNode*> njNode::get_children() const {
        return children_;
    }

    void njNode::add_child(njNode* node) {
        children_.push_back(node);
        node->parent_ = this;
        node->update_global_transform();
    }

    void njNode::remove_child(njNode* node) {
        const auto new_end{
            std::remove(children_.begin(), children_.end(), node)
        };
        if (new_end == children_.end()) {  // no elements were removed
            return;
        }
        children_.erase(new_end, children_.end());
        node->parent_ = nullptr;
        node->update_global_transform();
    }

    void njNode::set_local_transform(const math::njMat4f& local_transform) {
        local_transform_ = local_transform;
        update_global_transform();
    }

    void njNode::update_global_transform() {
        if (!parent_) {
            // if this is the root node
            // our global transform is the exact same as our local transform
            global_transform_ = local_transform_;
        } else {
            global_transform_ = parent_->global_transform_ * local_transform_;
        }

        for (auto* node : children_) {
            node->update_global_transform();
        }
    }

}  // namespace njin::core
