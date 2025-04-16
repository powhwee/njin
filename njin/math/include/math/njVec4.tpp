
namespace njin::math {
    template<typename T>
    njVec4<T>::njVec4() : x{ T{ 0 } }, y{ T{ 0 } }, z{ T{ 0 } }, w{ T{ 0 } } {}

    template<typename T>
    njVec4<T>::njVec4(T x, T y, T z, T w) : x{ x }, y{ y }, z{ z }, w{ w } {}

    template<typename T>
    njVec4<T>::njVec4(const std::array<T, 4>& array) :
        njVec4{ array[0], array[1], array[2], array[3] } {}

    template<typename T>
    T dot(const njVec4<T>& v1, const njVec4<T>& v2) {
        T result{};
        result += v1.x * v2.x;
        result += v1.y * v2.y;
        result += v1.z * v2.z;
        result += v1.w * v2.w;
        return result;
    }

    template<typename T>
    bool njVec4<T>::operator==(const njVec4<T>& other) const {
        bool result{ true };
        if (this->x != other.x) {
            result = false;
        }
        if (this->y != other.y) {
            result = false;
        }
        if (this->z != other.z) {
            result = false;
        }
        if (this->w != other.w) {
            result = false;
        }

        return result;
    }

    template<typename T>
    std::ostream& operator<<(std::ostream& os, const njVec4<T>& vec) {
        os << std::format("[{}, {}, {}, {}]", vec.x, vec.y, vec.z, vec.w);

        return os;
    }

};  // namespace njin::math
