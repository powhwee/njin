#include "math/njMat4.h"

#include <numbers>

#include <catch2/catch_test_macros.hpp>

#include <cmath>

namespace njin::math {
    TEST_CASE("njMat4", "[njMat4]") {
        njMat4<float> matrix{ { 0.0f, 1.0f, 2.0f, 3.0f },
                              { 4.0f, 5.0f, 6.0f, 7.0f },
                              { 8.0f, 9.0f, 10.0f, 11.0f },
                              { 12.0f, 13.0f, 14.0f, 15.0f } };

        SECTION("construction with njVec4<float> does not throw") {
            njVec4 vec_1{ 1.0f, 2.0f, 3.0f, 4.0f };
            njVec4<float> vec_2{ 1.0f, 2.0f, 3.0f, 4.0f };
            njVec4<float> vec_3{ 1.0f, 2.0f, 3.0f, 4.0f };
            njVec4<float> vec_4{ 1.0f, 2.0f, 3.0f, 4.0f };

            REQUIRE_NOTHROW(njMat4<float>{ vec_1, vec_2, vec_3, vec_4 });
        }

        SECTION("default construction") {
            njMat4<float> float_mat{};
            njMat4<float> float_expected{ { 1.f, 0.f, 0.f, 0.f },
                                          { 0.f, 1.f, 0.f, 0.f },
                                          { 0.f, 0.f, 1.f, 0.f },
                                          { 0.f, 0.f, 0.f, 1.f } };

            REQUIRE(float_mat == float_expected);

            njMat4<int> int_mat{};
            njMat4<int> int_expected{ { 1, 0, 0, 0 },
                                      { 0, 1, 0, 0 },
                                      { 0, 0, 1, 0 },
                                      { 0, 0, 0, 1 } };

            REQUIRE(int_mat == int_expected);
        }

        SECTION("addition") {
            njMat4<float> mat{ { 1.0f, 1.0f, 1.0f, 1.0f },
                               { 1.0f, 1.0f, 1.0f, 1.0f },
                               { 1.0f, 1.0f, 1.0f, 1.0f },
                               { 1.0f, 1.0f, 1.0f, 1.0f } };

            njMat4 result{ mat + mat };

            njMat4<float> expected{ { 2.0f, 2.0f, 2.0f, 2.0f },
                                    { 2.0f, 2.0f, 2.0f, 2.0f },
                                    { 2.0f, 2.0f, 2.0f, 2.0f },
                                    { 2.0f, 2.0f, 2.0f, 2.0f } };

            REQUIRE(result == expected);
        }

        SECTION("subtraction") {
            njMat4<float> mat{ { 1.0f, 1.0f, 1.0f, 1.0f },
                               { 1.0f, 1.0f, 1.0f, 1.0f },
                               { 1.0f, 1.0f, 1.0f, 1.0f },
                               { 1.0f, 1.0f, 1.0f, 1.0f } };

            njMat4<float> expected{ { 0.0f, 0.0f, 0.0f, 0.0f },
                                    { 0.0f, 0.0f, 0.0f, 0.0f },
                                    { 0.0f, 0.0f, 0.0f, 0.0f },
                                    { 0.0f, 0.0f, 0.0f, 0.0f } };

            njMat4 result{ mat - mat };

            REQUIRE(result == expected);
        }

        SECTION("multiplication") {
            njMat4<float> mat{ { 1.f, 2.f, 3.f, 4.f },
                               { 1.f, 2.f, 3.f, 4.f },
                               { 1.f, 2.f, 3.f, 4.f },
                               { 1.f, 2.f, 3.f, 4.f } };

            njMat4<float> expected{ { 10.f, 20.f, 30.f, 40.f },
                                    { 10.f, 20.f, 30.f, 40.f },
                                    { 10.f, 20.f, 30.f, 40.f },
                                    { 10.f, 20.f, 30.f, 40.f } };

            njMat4 result{ mat * mat };
            REQUIRE(result == expected);
        }

        SECTION("row retrieval") {
            njMat4<float> mat{ { 1.f, 2.f, 3.f, 4.f },
                               { 5.f, 6.f, 7.f, 8.f },
                               { 9.f, 10.f, 11.f, 12.f },
                               { 13.f, 14.f, 15.f, 16.f } };

            njVec4 expected{ 5.f, 6.f, 7.f, 8.f };
            njVec4 row{ mat.row(1) };

            REQUIRE(row == expected);
        }

        SECTION("column retrieval") {
            njMat4<float> mat{ { 1.f, 2.f, 3.f, 4.f },
                               { 5.f, 6.f, 7.f, 8.f },
                               { 9.f, 10.f, 11.f, 12.f },
                               { 13.f, 14.f, 15.f, 16.f } };

            njVec4 expected{ 2.f, 6.f, 10.f, 14.f };
            njVec4 col{ mat.col(1) };

            REQUIRE(col == expected);
        }

        SECTION("float matrix size is 16-byte aligned") {
            njMat4<float> mat{};

            size_t size{ sizeof(mat) };

            REQUIRE(size == 64);
        }

        SECTION("construction from translation vector") {
            njVec3f t{ 1.f, 2.f, 3.f };

            njMat4<float> mat{ njMat4Type::Translation, t };

            njMat4<float> expected{ { 1.f, 0.f, 0.f, 1.f },
                                    { 0.f, 1.f, 0.f, 2.f },
                                    { 0.f, 0.f, 1.f, 3.f },
                                    { 0.f, 0.f, 0.f, 1.f } };

            REQUIRE(mat == expected);
        }

        SECTION("construction from unit quaternion") {
            // rotation about the x-axis by 45 degrees
            float phi{ ((45.f / 2.f) * std::numbers::pi_v<float>) / 180.f };
            float sin{ std::sin(phi) };
            float cos{ std::cos(phi) };
            njVec4 r{ 1.f * sin, 0.f * sin, 0.f * sin, cos };

            njMat4<float> mat{ r };

            njMat4<float> expected{
                { 1.f, 0.f, 0.f, 0.f },
                { 0.f, 1 - 2 * (std::pow(sin, 2.f)), -2.f * cos * sin, 0.f },
                { 0.f, 2 * cos * sin, 1 - 2 * std::pow(sin, 2.f), 0.f },
                { 0.f, 0.f, 0.f, 1.f }
            };

            REQUIRE(mat == expected);
        }

        SECTION("construction from scale vector") {
            njVec3f s{ 1.f, 2.f, 3.f };

            njMat4<float> mat{ njMat4Type::Scale, s };
            njMat4<float> expected{ { 1.f, 0.f, 0.f, 0.f },
                                    { 0.f, 2.f, 0.f, 0.f },
                                    { 0.f, 0.f, 3.f, 0.f },
                                    { 0.f, 0.f, 0.f, 1.f } };

            REQUIRE(mat == expected);
        }
        SECTION("transpose") {
            njMat4f matrix{ { 1, 2, 3, 4 },
                            { 5, 6, 7, 8 },
                            { 9, 10, 11, 12 },
                            { 13, 14, 15, 16 } };

            njMat4f expected{ { 1, 5, 9, 13 },
                              { 2, 6, 10, 14 },
                              { 3, 7, 11, 15 },
                              { 4, 8, 12, 16 } };

            REQUIRE(transpose(matrix) == expected);
        }

        SECTION("orthogonality") {
            njMat4f orthogonal{ { 1, 0, 0, 0 },
                                { 0, 0, -1, 0 },
                                { 0, 1, 0, 0 },
                                { 0, 0, 0, 1 } };

            njMat4f not_orthogonal{ { 1, 2, 3, 4 },
                                    { 5, 6, 7, 8 },
                                    { 9, 10, 11, 12 },
                                    { 13, 14, 15, 16 } };

            REQUIRE(orthogonal.is_orthogonal());

            REQUIRE_FALSE(not_orthogonal.is_orthogonal());
        }

        SECTION("inverse") {
            njMat4f one{ { 0, -1, 0, 3 },
                         { 1, 0, 0, -2 },
                         { 0, 0, 1, 5 },
                         { 0, 0, 0, 1 } };
            REQUIRE(one * math::inverse(one) == njMat4f::Identity());

            njMat4f two{ { 2, 3, 1, 4 },
                         { 1, 2, 3, 5 },
                         { 3, 1, 2, 6 },
                         { 0, 0, 0, 1 } };
            REQUIRE(two * math::inverse(two) == njMat4f::Identity());
        }
        SECTION("component extraction") {
            SECTION("extract translation") {
                njVec3 translation{ matrix.get_translation_part() };
                njVec3 expected{ 3.f, 7.f, 11.f };
                REQUIRE(translation == expected);
            }

            SECTION("extract rotation / scale part") {
                using Row = njVec3f;
                using Matrix3 = std::array<Row, 3>;
                Row expected_row_0{ 0.f, 1.f, 2.f };
                Row expected_row_1{ 4.f, 5.f, 6.f };
                Row expected_row_2{ 8.f, 9.f, 10.f };

                Matrix3 expected{ expected_row_0,
                                  expected_row_1,
                                  expected_row_2 };
                Matrix3 rotation_scale{ matrix.get_rotation_scale_part() };

                REQUIRE(expected == rotation_scale);
            }
        }
    }

}  // namespace njin::math