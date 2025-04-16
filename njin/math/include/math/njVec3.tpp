#include <format>

namespace njin::math {
    template<typename T>
    njVec3<T>::njVec3(T x, T y, T z) : x{ x }, y{ y }, z{ z } {}

    template<typename T>
    njVec3<T> njVec3<T>::zero() {
        return { 0, 0, 0 };
    }

    template<typename T>
    std::ostream& operator<<(std::ostream& os, const njVec3<T>& vec) {
        os << std::format("({}, {}, {})", vec.x, vec.y, vec.z);
        return os;
    }

    template<typename T>
    njVec3<T> njVec3<T>::operator-(const njVec3<T>& other) const {
        return { x - other.x, y - other.y, z - other.z };
    }

    template<typename T>
    njVec3<T> njVec3<T>::operator+(const njVec3<T>& other) const {
        return { x + other.x, y + other.y, z + other.z };
    }

    template<typename T>
    njVec3<T> cross(const njVec3<T>& a, const njVec3<T>& b) {
        const float x{ a.y * b.z - a.z * b.y };
        const float y{ a.z * b.x - a.x * b.z };
        const float z{ a.x * b.y - a.y * b.x };
        return { x, y, z };
    }

    template<typename T>
    njVec3<T> normalize(const njVec3<T>& vector) {
        using namespace std;
        const float scalar{ sqrtf((powf(vector.x, 2) + powf(vector.y, 2) +
                                   powf(vector.z, 2))) };

        return { vector.x / scalar, vector.y / scalar, vector.z / scalar };
    }

    template<typename T>
    bool njVec3<T>::operator==(const njVec3<T>& other) const {
        float epsilon{ 1e-5 };
        bool result{ true };
        if (std::abs(x - other.x) > epsilon) {
            result = false;
        }
        if (std::abs(y - other.y) > epsilon) {
            result = false;
        }
        if (std::abs(z - other.z) > epsilon) {
            result = false;
        }
        return result;
    }

    template<typename T>
    njVec3<T> njVec3<T>::operator-() const {
        return { -x, -y, -z };
    }

    template<typename T>
    njVec3<T> njVec3<T>::operator*(const njVec3<T>& other) const {
        return { x * other.x, y * other.y, z * other.z };
    }

    template<typename T>
    njVec3<T> njVec3<T>::operator*(T scalar) const {
        return { x * scalar, y * scalar, z * scalar };
    }

    template<typename T>
    T magnitude(const njVec3<T>& vector) {
        return std::sqrt(vector.x * vector.x + vector.y * vector.y +
                         vector.z * vector.z);
    }

    template<typename T>
    T& njVec3<T>::operator[](int index) {
        switch (index) {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        default:
            throw std::runtime_error("invalid index passed to vec3");
        }
    }

    template<typename T>
    T njVec3<T>::operator[](int index) const {
        switch (index) {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        default:
            throw std::runtime_error("invalid index passed to vec3");
        }
    }

}  // namespace njin::math
