#pragma once
#include "math/njVec3.h"

#include <cmath>

namespace njin::math {
    template<typename T>
    class njQuat;

    template<typename T>
    class njQuat {
        public:
        T x;  // i component
        T y;  // j component
        T z;  // k component
        T w;  // scalar/real component

        // Identity quaternion
        njQuat() : x(0), y(0), z(0), w(1) {}

        njQuat(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

        // Create from axis-angle
        static njQuat from_axis_angle(const njVec3<T>& axis, T radians) {
            T half_angle = radians / T(2);
            T s = std::sin(half_angle);
            njVec3<T> n = normalize(axis);
            return njQuat(n.x * s, n.y * s, n.z * s, std::cos(half_angle));
        }

        // Normalize quaternion
        njQuat normalized() const {
            T len = std::sqrt(x * x + y * y + z * z + w * w);
            if (len < T(0.0001))
                return njQuat();
            return njQuat(x / len, y / len, z / len, w / len);
        }

        // Quaternion multiplication
        njQuat operator*(const njQuat& other) const {
            return njQuat(w * other.x + x * other.w + y * other.z - z * other.y,
                          w * other.y - x * other.z + y * other.w + z * other.x,
                          w * other.z + x * other.y - y * other.x + z * other.w,
                          w * other.w - x * other.x - y * other.y -
                          z * other.z);
        }

        bool operator==(const njQuat& other) const {
            return x == other.x && y == other.y && z == other.z && w == other.w;
        }
    };

    // Spherical linear interpolation
    template<typename T>
    njQuat<T> slerp(const njQuat<T>& q1, const njQuat<T>& q2, T t) {
        // Compute cosine of angle between quaternions
        T cos_theta = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;

        // If cos_theta < 0, negate one quaternion to take shorter path
        njQuat<T> q2_adj = q2;
        if (cos_theta < T(0)) {
            q2_adj = njQuat<T>(-q2.x, -q2.y, -q2.z, -q2.w);
            cos_theta = -cos_theta;
        }

        // If quaternions are very close, use linear interpolation
        if (cos_theta > T(0.9995)) {
            return njQuat<T>(q1.x + t * (q2_adj.x - q1.x),
                             q1.y + t * (q2_adj.y - q1.y),
                             q1.z + t * (q2_adj.z - q1.z),
                             q1.w + t * (q2_adj.w - q1.w))
            .normalized();
        }

        T theta = std::acos(cos_theta);
        T sin_theta = std::sin(theta);

        T w1 = std::sin((T(1) - t) * theta) / sin_theta;
        T w2 = std::sin(t * theta) / sin_theta;

        return njQuat<T>(w1 * q1.x + w2 * q2_adj.x,
                         w1 * q1.y + w2 * q2_adj.y,
                         w1 * q1.z + w2 * q2_adj.z,
                         w1 * q1.w + w2 * q2_adj.w);
    }

    using njQuatf = njQuat<float>;
}  // namespace njin::math
