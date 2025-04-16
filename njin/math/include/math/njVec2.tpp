namespace njin::math {
    template<typename T>
    njVec2<T>::njVec2(T x, T y) : x{ x }, y{ y } {}

    template<typename T>
    bool njVec2<T>::operator==(const njVec2<T>& other) const {
        return x == other.x && y == other.y;
    }
}  // namespace njin::math