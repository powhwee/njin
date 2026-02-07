#pragma once
#include <string>
#include <variant>
#include <vector>

#include "math/njQuat.h"
#include "math/njVec3.h"

namespace njin::core {

    struct njKeyframe {
        float time;
        std::variant<math::njVec3f, math::njQuatf>
        value;  // Translation/Scale (Vec3), Rotation (Quat)
    };

    struct njAnimationChannel {
        int target_node;  // node index this channel affects
        enum class Path {
            Translation,
            Rotation,
            Scale
        } path;
        std::vector<njKeyframe> keyframes;
    };

    struct njAnimation {
        std::string name;
        float duration;  // max time
        std::vector<njAnimationChannel> channels;
    };

}  // namespace njin::core
