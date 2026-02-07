#include "util/Accessor.h"

namespace njin::gltf {

    Accessor::Accessor(const AccessorCreateInfo& info) :
        elements_{ [&]() -> std::vector<Element> {
            std::vector<Element> elements{};

            std::vector<std::byte> bytes{ info.buffer_view.get() };

            size_t element_size = 0;
            if (info.type == Type::Vec3 &&
                info.component_type == ComponentType::Float) {
                element_size = sizeof(float) * 3;
            } else if (info.type == Type::Vec4 &&
                       info.component_type == ComponentType::Float) {
                element_size = sizeof(float) * 4;
            } else if (info.type == Type::Vec2 &&
                       info.component_type == ComponentType::Float) {
                element_size = sizeof(float) * 2;
            } else if (info.type == Type::Scalar &&
                       info.component_type == ComponentType::UnsignedInt) {
                element_size = sizeof(uint32_t);
            } else if (info.type == Type::Scalar &&
                       info.component_type == ComponentType::UnsignedShort) {
                element_size = sizeof(uint16_t);
            } else if (info.type == Type::Scalar &&
                       info.component_type == ComponentType::Float) {
                element_size = sizeof(float);
            } else if (info.type == Type::Vec4 &&
                       info.component_type == ComponentType::UnsignedShort) {
                element_size = sizeof(uint16_t) * 4;
            } else if (info.type == Type::Mat4 &&
                       info.component_type == ComponentType::Float) {
                element_size = sizeof(float) * 16;
            } else if (info.type == Type::Vec4 &&
                       info.component_type == ComponentType::UnsignedByte) {
                element_size = sizeof(uint8_t) * 4;
            }

            uint32_t stride =
            info.buffer_view.get_byte_stride().value_or(element_size);

            uint32_t current_offset{ info.byte_offset };
            for (uint32_t i = 0; i < info.count; ++i) {
                if (info.type == Type::Vec3 &&
                    info.component_type == ComponentType::Float) {
                    float c1, c2, c3;
                    std::memcpy(&c1,
                                bytes.data() + current_offset,
                                sizeof(float));
                    std::memcpy(&c2,
                                bytes.data() + current_offset + sizeof(float),
                                sizeof(float));
                    std::memcpy(&c3,
                                bytes.data() + current_offset +
                                2 * sizeof(float),
                                sizeof(float));
                    elements.push_back(math::njVec3f{ c1, c2, c3 });
                } else if (info.type == Type::Vec4 &&
                           info.component_type == ComponentType::Float) {
                    float c1, c2, c3, c4;
                    std::memcpy(&c1,
                                bytes.data() + current_offset,
                                sizeof(float));
                    std::memcpy(&c2,
                                bytes.data() + current_offset + sizeof(float),
                                sizeof(float));
                    std::memcpy(&c3,
                                bytes.data() + current_offset +
                                2 * sizeof(float),
                                sizeof(float));
                    std::memcpy(&c4,
                                bytes.data() + current_offset +
                                3 * sizeof(float),
                                sizeof(float));
                    elements.push_back(math::njVec4f{ c1, c2, c3, c4 });
                } else if (info.type == Type::Vec2 &&
                           info.component_type == ComponentType::Float) {
                    float c1, c2;
                    std::memcpy(&c1,
                                bytes.data() + current_offset,
                                sizeof(float));
                    std::memcpy(&c2,
                                bytes.data() + current_offset + sizeof(float),
                                sizeof(float));
                    elements.push_back(math::njVec2f{ c1, c2 });
                } else if (info.type == Type::Scalar &&
                           info.component_type == ComponentType::UnsignedInt) {
                    uint32_t c1;
                    std::memcpy(&c1,
                                bytes.data() + current_offset,
                                sizeof(uint32_t));
                    elements.push_back(c1);
                } else if (info.type == Type::Scalar &&
                           info.component_type ==
                           ComponentType::UnsignedShort) {
                    uint16_t c1;
                    std::memcpy(&c1,
                                bytes.data() + current_offset,
                                sizeof(uint16_t));
                    elements.push_back(c1);
                } else if (info.type == Type::Scalar &&
                           info.component_type == ComponentType::Float) {
                    float c1;
                    std::memcpy(&c1,
                                bytes.data() + current_offset,
                                sizeof(float));
                    elements.push_back(c1);
                } else if (info.type == Type::Vec4 &&
                           info.component_type ==
                           ComponentType::UnsignedShort) {
                    uint16_t c1, c2, c3, c4;
                    std::memcpy(&c1,
                                bytes.data() + current_offset,
                                sizeof(uint16_t));
                    std::memcpy(&c2,
                                bytes.data() + current_offset +
                                sizeof(uint16_t),
                                sizeof(uint16_t));
                    std::memcpy(&c3,
                                bytes.data() + current_offset +
                                2 * sizeof(uint16_t),
                                sizeof(uint16_t));
                    std::memcpy(&c4,
                                bytes.data() + current_offset +
                                3 * sizeof(uint16_t),
                                sizeof(uint16_t));
                    elements
                    .push_back(math::njVec4<uint16_t>{ c1, c2, c3, c4 });
                } else if (info.type == Type::Mat4 &&
                           info.component_type == ComponentType::Float) {
                    // Read 16 floats (column-major in glTF)
                    float m[16];
                    for (int j = 0; j < 16; ++j) {
                        std::memcpy(&m[j],
                                    bytes.data() + current_offset +
                                    j * sizeof(float),
                                    sizeof(float));
                    }
                    // Transpose column-major → row-major for njMat4
                    // glTF: indices 0-3 = column 0, 4-7 = column 1, ...
                    // njMat4 rows: row0 = {col0[0], col1[0], col2[0], col3[0]}
                    math::njVec4f r0{ m[0], m[4], m[8], m[12] };
                    math::njVec4f r1{ m[1], m[5], m[9], m[13] };
                    math::njVec4f r2{ m[2], m[6], m[10], m[14] };
                    math::njVec4f r3{ m[3], m[7], m[11], m[15] };
                    elements.push_back(math::njMat4f(r0, r1, r2, r3));
                } else if (info.type == Type::Vec4 &&
                           info.component_type == ComponentType::UnsignedByte) {
                    // JOINTS_0 sometimes stored as uint8, promote to uint16
                    uint8_t c1, c2, c3, c4;
                    std::memcpy(&c1,
                                bytes.data() + current_offset,
                                sizeof(uint8_t));
                    std::memcpy(&c2,
                                bytes.data() + current_offset + sizeof(uint8_t),
                                sizeof(uint8_t));
                    std::memcpy(&c3,
                                bytes.data() + current_offset +
                                2 * sizeof(uint8_t),
                                sizeof(uint8_t));
                    std::memcpy(&c4,
                                bytes.data() + current_offset +
                                3 * sizeof(uint8_t),
                                sizeof(uint8_t));
                    elements.push_back(math::njVec4<uint16_t>{
                        static_cast<uint16_t>(c1),
                        static_cast<uint16_t>(c2),
                        static_cast<uint16_t>(c3),
                        static_cast<uint16_t>(c4) });
                }

                current_offset += stride;
            }

            return elements;
        }() } {}

    std::vector<math::njVec2f> Accessor::get_vec2f() const {
        std::vector<math::njVec2f> elements{};
        for (const auto& element : elements_) {
            elements.push_back(std::get<math::njVec2f>(element));
        }

        return elements;
    }

    std::vector<math::njVec3f> Accessor::get_vec3f() const {
        std::vector<math::njVec3f> elements{};
        for (const auto& element : elements_) {
            elements.push_back(std::get<math::njVec3f>(element));
        }

        return elements;
    }

    std::vector<math::njVec4<float>> Accessor::get_vec4f() const {
        std::vector<math::njVec4<float>> elements{};
        for (const auto& element : elements_) {
            elements.push_back(std::get<math::njVec4<float>>(element));
        }

        return elements;
    }

    std::vector<math::njVec4<uint16_t>> Accessor::get_vec4ushort() const {
        std::vector<math::njVec4<uint16_t>> elements{};
        for (const auto& element : elements_) {
            elements.push_back(std::get<math::njVec4<uint16_t>>(element));
        }

        return elements;
    }

    std::vector<uint16_t> Accessor::get_scalar() const {
        std::vector<uint16_t> elements{};
        for (const auto& element : elements_) {
            elements.push_back(std::get<uint16_t>(element));
        }

        return elements;
    }

    std::vector<float> Accessor::get_scalar_f() const {
        std::vector<float> elements{};
        for (const auto& element : elements_) {
            elements.push_back(std::get<float>(element));
        }

        return elements;
    }

    std::vector<uint32_t> Accessor::get_scalar_u32() const {
        std::vector<uint32_t> elements{};
        for (const auto& element : elements_) {
            elements.push_back(std::get<uint32_t>(element));
        }

        return elements;
    }

    std::vector<math::njMat4f> Accessor::get_mat4f() const {
        std::vector<math::njMat4f> elements{};
        for (const auto& element : elements_) {
            elements.push_back(std::get<math::njMat4f>(element));
        }

        return elements;
    }

    std::vector<math::njVec4<uint16_t>> Accessor::get_vec4_ubyte() const {
        // Elements were already promoted to uint16 during parsing
        std::vector<math::njVec4<uint16_t>> elements{};
        for (const auto& element : elements_) {
            elements.push_back(std::get<math::njVec4<uint16_t>>(element));
        }

        return elements;
    }

}  // namespace njin::gltf