#include "njVec3.h"

#include <cmath>

namespace njin::math {
    template<typename T>
    njMat4<T>::njMat4() :
        njMat4{ { 1, 0, 0, 0 },
                { 0, 1, 0, 0 },
                { 0, 0, 1, 0 },
                { 0, 0, 0, 1 } } {}

    template<typename T>
    njMat4<T>::njMat4(const njVec4<T>& row_1,
                      const njVec4<T>& row_2,
                      const njVec4<T>& row_3,
                      const njVec4<T>& row_4) {
        matrix_[0][0] = row_1.x;
        matrix_[0][1] = row_1.y;
        matrix_[0][2] = row_1.z;
        matrix_[0][3] = row_1.w;

        matrix_[1][0] = row_2.x;
        matrix_[1][1] = row_2.y;
        matrix_[1][2] = row_2.z;
        matrix_[1][3] = row_2.w;

        matrix_[2][0] = row_3.x;
        matrix_[2][1] = row_3.y;
        matrix_[2][2] = row_3.z;
        matrix_[2][3] = row_3.w;

        matrix_[3][0] = row_4.x;
        matrix_[3][1] = row_4.y;
        matrix_[3][2] = row_4.z;
        matrix_[3][3] = row_4.w;
    }

    template<typename T>
    njMat4<T>::njMat4(njMat4Type type, const njVec3f& data) {
        Row row_1{};
        Row row_2{};
        Row row_3{};
        Row row_4{};
        if (type == njMat4Type::Translation) {
            row_1 = { 1.f, 0.f, 0.f, data.x };
            row_2 = { 0.f, 1.f, 0.f, data.y };
            row_3 = { 0.f, 0.f, 1.f, data.z };
            row_4 = { 0.f, 0.f, 0.f, 1.f };
        } else if (type == njMat4Type::Scale) {
            row_1 = { data.x, 0.f, 0.f, 0.f };
            row_2 = { 0.f, data.y, 0.f, 0.f };
            row_3 = { 0.f, 0.f, data.z, 0.f };
            row_4 = { 0.f, 0.f, 0.f, 1.f };
        }

        matrix_[0] = row_1;
        matrix_[1] = row_2;
        matrix_[2] = row_3;
        matrix_[3] = row_4;
    }

    template<typename T>
    njMat4<T>::njMat4(const njVec4f& quaternion) {
        // njVec4 is a unit quaternion
        float qx{ quaternion.x };
        float qy{ quaternion.y };
        float qz{ quaternion.z };
        float qw{ quaternion.w };

        Row row_0{ 1 - 2 * (std::pow(qy, 2.f) + std::pow(qz, 2.f)),
                   2 * (qx * qy - qw * qz),
                   2 * (qx * qz + qw * qy),
                   0 };

        Row row_1{ 2 * (qx * qy + qw * qz),
                   1 - 2 * (std::pow(qx, 2.f) + std::pow(qz, 2.f)),
                   2 * (qy * qz - qw * qx),
                   0 };

        Row row_2{ 2 * (qx * qz - qw * qy),
                   2 * (qy * qz + qw * qx),
                   1 - 2 * (std::pow(qx, 2.f) + std::pow(qy, 2.f)),
                   0 };

        Row row_3{ 0, 0, 0, 1 };

        matrix_[0] = row_0;
        matrix_[1] = row_1;
        matrix_[2] = row_2;
        matrix_[3] = row_3;
    }

    template<typename T>
    njMat4<T> njMat4<T>::operator+(const njMat4& other) const {
        njMat4<T> result{};
        for (int i{ 0 }; i < 4; ++i) {
            for (int j{ 0 }; j < 4; ++j) {
                result.matrix_[i][j] = this->matrix_[i][j] +
                                       other.matrix_[i][j];
            }
        }
        return result;
    };

    template<typename T>
    bool njMat4<T>::operator==(const njMat4& other) const {
        for (int i{ 0 }; i < 4; ++i) {
            for (int j{ 0 }; j < 4; ++j) {
                if (std::abs(this->matrix_[i][j] - other.matrix_[i][j]) >
                    std::numeric_limits<T>::epsilon()) {
                    return false;
                }
            }
        }

        return true;
    }

    template<typename T>
    njMat4<T> njMat4<T>::operator-(const njMat4& other) const {
        njMat4<T> result{};
        for (int i{ 0 }; i < 4; ++i) {
            for (int j{ 0 }; j < 4; ++j) {
                result.matrix_[i][j] = this->matrix_[i][j] -
                                       other.matrix_[i][j];
            }
        }
        return result;
    }

    template<typename T>
    njMat4<T> njMat4<T>::operator*(const njMat4& other) const {
        njMat4<T> result{};
        for (int i{ 0 }; i < 4; ++i) {
            for (int j{ 0 }; j < 4; ++j) {
                njVec4<T> row{ this->row(i) };
                njVec4<T> col{ other.col(j) };

                result.matrix_[i][j] = dot(row, col);
            }
        }

        return result;
    }

    template<typename T>
    njVec4<T> njMat4<T>::row(int row_index) const {
        return matrix_[row_index];
    }

    template<typename T>
    njVec4<T> njMat4<T>::col(int col_index) const {
        njVec4<T> result{};
        result.x = matrix_[0][col_index];
        result.y = matrix_[1][col_index];
        result.z = matrix_[2][col_index];
        result.w = matrix_[3][col_index];

        return result;
    }

    template<typename T>
    njMat4<T> njMat4<T>::Identity() {
        return { { 1, 0, 0, 0 },
                 { 0, 1, 0, 0 },
                 { 0, 0, 1, 0 },
                 { 0, 0, 0, 1 } };
    }

    template<typename T>
    njMat4<T> inverse(const njMat4<T>& matrix) {
        auto& m{ matrix };
        // if a matrix is orthogonal its inverse is its transpose
        if (matrix.is_orthogonal()) {
            return transpose(matrix);
        } else if (false) {
            // TODO: rigidbody inverse
            return {};
        } else {
            // general inverse
            T s0{ m[0][0] * m[1][1] - m[1][0] * m[0][1] };
            T s1 = m[0][0] * m[1][2] - m[1][0] * m[0][2];
            T s2 = m[0][0] * m[1][3] - m[1][0] * m[0][3];
            T s3 = m[0][1] * m[1][2] - m[1][1] * m[0][2];
            T s4 = m[0][1] * m[1][3] - m[1][1] * m[0][3];
            T s5 = m[0][2] * m[1][3] - m[1][2] * m[0][3];

            T c5 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
            T c4 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
            T c3 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
            T c2 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
            T c1 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
            T c0 = m[2][0] * m[3][1] - m[3][0] * m[2][1];

            // Should check for 0 determinant
            T inv_det = 1.0 / (s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 +
                               s5 * c0);

            njMat4<T> result{};

            result[0][0] = (m[1][1] * c5 - m[1][2] * c4 + m[1][3] * c3) *
                           inv_det;
            result[0][1] = (-m[0][1] * c5 + m[0][2] * c4 - m[0][3] * c3) *
                           inv_det;
            result[0][2] = (m[3][1] * s5 - m[3][2] * s4 + m[3][3] * s3) *
                           inv_det;
            result[0][3] = (-m[2][1] * s5 + m[2][2] * s4 - m[2][3] * s3) *
                           inv_det;

            result[1][0] = (-m[1][0] * c5 + m[1][2] * c2 - m[1][3] * c1) *
                           inv_det;
            result[1][1] = (m[0][0] * c5 - m[0][2] * c2 + m[0][3] * c1) *
                           inv_det;
            result[1][2] = (-m[3][0] * s5 + m[3][2] * s2 - m[3][3] * s1) *
                           inv_det;
            result[1][3] = (m[2][0] * s5 - m[2][2] * s2 + m[2][3] * s1) *
                           inv_det;

            result[2][0] = (m[1][0] * c4 - m[1][1] * c2 + m[1][3] * c0) *
                           inv_det;
            result[2][1] = (-m[0][0] * c4 + m[0][1] * c2 - m[0][3] * c0) *
                           inv_det;
            result[2][2] = (m[3][0] * s4 - m[3][1] * s2 + m[3][3] * s0) *
                           inv_det;
            result[2][3] = (-m[2][0] * s4 + m[2][1] * s2 - m[2][3] * s0) *
                           inv_det;

            result[3][0] = (-m[1][0] * c3 + m[1][1] * c1 - m[1][2] * c0) *
                           inv_det;
            result[3][1] = (m[0][0] * c3 - m[0][1] * c1 + m[0][2] * c0) *
                           inv_det;
            result[3][2] = (-m[3][0] * s3 + m[3][1] * s1 - m[3][2] * s0) *
                           inv_det;
            result[3][3] = (m[2][0] * s3 - m[2][1] * s1 + m[2][2] * s0) *
                           inv_det;

            return result;
        }
    }

    template<typename T>
    njMat4<T> transpose(const njMat4<T>& matrix) {
        auto& m = matrix;
        njMat4<T> result{ { m[0][0], m[1][0], m[2][0], m[3][0] },
                          { m[0][1], m[1][1], m[2][1], m[3][1] },
                          { m[0][2], m[1][2], m[2][2], m[3][2] },
                          { m[0][3], m[1][3], m[2][3], m[3][3] } };
        return result;
    }

    template<typename T>
    std::array<T, 4>& njMat4<T>::operator[](size_t row_index) {
        return matrix_[row_index];
    }

    template<typename T>
    const std::array<T, 4>& njMat4<T>::operator[](size_t row_index) const {
        return matrix_[row_index];
    }

    template<typename T>
    bool njMat4<T>::is_orthogonal() const {
        // a matrix is orthogonal if its transpose is equal to its inverse
        njMat4<T> transpose{ math::transpose(*this) };
        return { transpose * (*this) == Identity() };
    }

    template<typename T>
    njVec3<T> njMat4<T>::get_translation_part() const {
        njVec4<T> last_col{ col(3) };
        return { last_col.x, last_col.y, last_col.z };
    }

    template<typename T>
    std::array<njVec3<T>, 3> njMat4<T>::get_rotation_scale_part() const {
        njVec3<T> row_0{ row(0).x, row(0).y, row(0).z };
        njVec3<T> row_1{ row(1).x, row(1).y, row(1).z };
        njVec3<T> row_2{ row(2).x, row(2).y, row(2).z };

        return { row_0, row_1, row_2 };
    }

    template<typename T>
    std::ostream& operator<<(std::ostream& os, const njMat4<T>& mat) {
        std::cout << std::endl
                  << std::format("[{}, {}, {}, {}]",
                                 mat[0][0],
                                 mat[0][1],
                                 mat[0][2],
                                 mat[0][3])
                  << std::endl;
        std::cout << std::format("[{}, {}, {}, {}]",
                                 mat[1][0],
                                 mat[1][1],
                                 mat[1][2],
                                 mat[1][3])
                  << std::endl;
        std::cout << std::format("[{}, {}, {}, {}]",
                                 mat[2][0],
                                 mat[2][1],
                                 mat[2][2],
                                 mat[2][3])
                  << std::endl;
        std::cout << std::format("[{}, {}, {}, {}]",
                                 mat[3][0],
                                 mat[3][1],
                                 mat[3][2],
                                 mat[3][3]);
        return os;
    }

}  // namespace njin::math
