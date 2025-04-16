#pragma once

namespace njin::math {
    template<typename T>
    class njVec2 {
        public:
            njVec2(T x, T y);
            njVec2() = default;

            T x;
            T y;

            bool operator==(const njVec2<T>& other) const;
    };

    using njVec2f = njVec2<float>;
}  // namespace njin::math

#include "math/njVec2.tpp"