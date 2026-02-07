#pragma once
#include <string>
#include <vector>

#include "math/njMat4.h"
#include "math/njQuat.h"
#include "math/njVec3.h"

namespace njin::core {

    struct njSkeletonNode {
        std::string name;
        int parent;                     // -1 if root
        math::njMat4f local_transform;  // bind pose matrix (cached)

        // Bind pose components for animation blending
        math::njVec3f bind_translation;
        math::njQuatf bind_rotation;
        math::njVec3f bind_scale;

        std::vector<int> children;  // indices of children
        int mesh_index;             // -1 if no mesh
        std::string mesh_name;      // name of mesh for registry lookup
    };

    struct njSkeleton {
        std::vector<njSkeletonNode> nodes;
        std::vector<int> root_nodes;

        // Skin data for GPU skinning
        std::vector<int> joint_nodes;  // glTF node indices for each joint
        std::vector<math::njMat4f> inverse_bind_matrices;  // per-joint IBMs
    };

}  // namespace njin::core
