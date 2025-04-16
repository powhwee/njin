#pragma once
#include <array>
#include <format>

#include "math/njVec3.h"

namespace njin::math {
    template<typename T>
    class njVec4 {
        public:
        njVec4();

        njVec4(T x, T y, T z, T w);

        njVec4(const std::array<T, 4>& array);

        bool operator==(const njVec4<T>& other) const;

        T x;
        T y;
        T z;
        T w;

        friend std::ostream& operator<< <>(std::ostream& os,
                                           const njVec4<T>& vec);
    };

    template<typename T>
    T dot(const njVec4<T>& v1, const njVec4<T>& v2);


    using njVec4f = njVec4<float>;
}  // namespace njin::math

#include "math/njVec4.tpp"
