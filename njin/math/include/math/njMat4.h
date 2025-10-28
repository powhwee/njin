#pragma once
#include <array>

#include "math.h"
#include "math/njVec3.h"
#include "math/njVec4.h"

namespace njin::math {
    enum class njMat4Type {
        Translation,
        Scale
    };

    template<typename T>
    class njMat4;

    template<typename T>
    std::ostream& operator<<(std::ostream& os, const njMat4<T>& vec);

    template<typename T>
    class njMat4 {
        public:
        njMat4();

        static njMat4<T> Identity();

        /**
         * From four row vectors
         * @param row_0 Zeroth row vector
         * @param row_1 First row vector
         * @param row_2 Second row vector
         * @param row_3 Third row vector
         */
        njMat4(const njVec4<T>& row_0,
               const njVec4<T>& row_1,
               const njVec4<T>& row_2,
               const njVec4<T>& row_3);

        /**
         * From either a translation or scale vector
         * @param type Type of vector
         * @param data Vector data
         * @note This vector must be a 3D vector in the local coordinate system
         * @note Construction of these two kinds of matrices are grouped
         * together because they are both njVec3
         */
        njMat4(njMat4Type type, const njVec3f& data);

        /**
         * From a unit quaternion
         * @param quaternion Unit quaternion
         * @note The imaginary part must be relative to the local coordinate system
         */
        explicit njMat4(const njVec4f& quaternion);

        njMat4<T> operator+(const njMat4& other) const;

        njMat4<T> operator-(const njMat4& other) const;

        njMat4<T> operator*(const njMat4& other) const;

        /**
         * Retrieve a row of this njMat4
         * @param row_index 0-indexed row of matrix
         * @return Row of matrix
         */
        njVec4<T> row(int row_index) const;

        /**
         * Retrieve a column of this njMat4
         * @param col_index 0-indexed column of matrix
         * @return Column of matrix
         */
        njVec4<T> col(int col_index) const;

        bool operator==(const njMat4& other) const;

        std::array<T, 4>& operator[](size_t row_index);

        const std::array<T, 4>& operator[](size_t row_index) const;

        /**
         * Check if this matrix is orthogonal
         * @return True if the matrix is orthogonal
         */
        bool is_orthogonal() const;

        friend std::ostream& operator<< <>(std::ostream& os,
                                           const njMat4<T>& vec);

        /**
         * Assuming this matrix is a transform matrix, retrieve the part
         * that represents the translation (last column)
         * @return Translation vector
         */
        njVec3<T> get_translation_part() const;

        /**
         * Assuming this matrix is a transform matrix, retrieve
         * the part that represents the rotation and scale (upper left 3x3)
         * @return Rotation / Scale matrix
         */
        std::array<njVec3<T>, 3> get_rotation_scale_part() const;

        private:
        using Row = std::array<T, 4>;
        using Matrix = std::array<Row, 4>;

        Matrix matrix_{};
    };

    template<typename T>
    njMat4<T> inverse(const njMat4<T>& matrix);

    template<typename T>
    njMat4<T> transpose(const njMat4<T>& matrix);

    // NOTE: maybe move this somewhere outside of njMat4.h / njVec4.h?
    /**
     * Multiply a 4x4 matrix by a 4x1 vector
     * @tparam T Element type
     * @param m Matrix to pre-multiply
     * @param v Vector to multiply with
     * @return Resulting 4x1 vector
     */
    template<typename T>
    njVec4<T> operator*(const njMat4<T>& m, const njVec4<T>& v) {
        T x{ v.x * m[0][0] + v.y * m[0][1] + v.z * m[0][2] + v.w * m[0][3] };
        T y{ v.x * m[1][0] + v.y * m[1][1] + v.z * m[1][2] + v.w * m[1][3] };
        T z{ v.x * m[2][0] + v.y * m[2][1] + v.z * m[2][2] + v.w * m[2][3] };
        T w{ v.x * m[3][0] + v.y * m[3][1] + v.z * m[3][2] + v.w * m[3][3] };
        return { x, y, z, w };
    }

    using njMat4f = njMat4<float>;
}  // namespace njin::math

#include "math/njMat4.tpp"
