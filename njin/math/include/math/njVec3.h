#pragma once
#include <iostream>

namespace njin::math {
    template<typename T>
    class njVec3;

    template<typename T>
    std::ostream& operator<<(std::ostream& os, const njVec3<T>& vec);

    template<typename T>
    class njVec3 {
        public:
        njVec3(T x, T y, T z);

        njVec3() = default;

        static njVec3 zero();

        T x;
        T y;
        T z;

        njVec3<T> operator-(const njVec3<T>& other) const;

        njVec3<T> operator-() const;

        njVec3<T> operator*(const njVec3<T>& other) const;



        njVec3<T> operator*(T scalar) const;

        njVec3<T> operator+(const njVec3<T>& other) const;

        T& operator[](int index);

        T operator[](int index) const;

        bool operator==(const njVec3<T>& other) const;

        friend std::ostream& operator<< <>(std::ostream& os,
                                           const njVec3<T>& vec);
    };

    template<typename T>
    njVec3<T> cross(const njVec3<T>& a, const njVec3<T>& b);

    template<typename T>
    njVec3<T> normalize(const njVec3<T>& vector);

    template<typename T>
    T magnitude(const njVec3<T>& vector);

    using njVec3f = njVec3<float>;
}  // namespace njin::math

#include "math/njVec3.tpp"
