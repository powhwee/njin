#include "util/Accessor.h"

namespace njin::gltf {

    Accessor::Accessor(const AccessorCreateInfo& info) :
        elements_{ [&]() -> std::vector<Element> {
            std::vector<Element> elements{};

            std::vector<std::byte> bytes{ info.buffer_view.get() };

            size_t element_size = 0;
            if (info.type == Type::Vec3 && info.component_type == ComponentType::Float) {
                element_size = sizeof(float) * 3;
            } else if (info.type == Type::Vec4 && info.component_type == ComponentType::Float) {
                element_size = sizeof(float) * 4;
            } else if (info.type == Type::Vec2 && info.component_type == ComponentType::Float) {
                element_size = sizeof(float) * 2;
            } else if (info.type == Type::Scalar && info.component_type == ComponentType::UnsignedInt) {
                element_size = sizeof(uint32_t);
            } else if (info.type == Type::Scalar && info.component_type == ComponentType::UnsignedShort) {
                element_size = sizeof(uint16_t);
            } else if (info.type == Type::Vec4 && info.component_type == ComponentType::UnsignedShort) {
                element_size = sizeof(uint16_t) * 4;
            }

            uint32_t stride = info.buffer_view.get_byte_stride().value_or(element_size);

            uint32_t current_offset{ info.byte_offset };
            for (uint32_t i = 0; i < info.count; ++i) {
                if (info.type == Type::Vec3 &&
                    info.component_type == ComponentType::Float) {
                    float c1, c2, c3;
                    std::memcpy(&c1, bytes.data() + current_offset, sizeof(float));
                    std::memcpy(&c2, bytes.data() + current_offset + sizeof(float), sizeof(float));
                    std::memcpy(&c3, bytes.data() + current_offset + 2 * sizeof(float), sizeof(float));
                    elements.push_back(math::njVec3f{ c1, c2, c3 });
                } else if (info.type == Type::Vec4 &&
                           info.component_type == ComponentType::Float) {
                    float c1, c2, c3, c4;
                    std::memcpy(&c1, bytes.data() + current_offset, sizeof(float));
                    std::memcpy(&c2, bytes.data() + current_offset + sizeof(float), sizeof(float));
                    std::memcpy(&c3, bytes.data() + current_offset + 2 * sizeof(float), sizeof(float));
                    std::memcpy(&c4, bytes.data() + current_offset + 3 * sizeof(float), sizeof(float));
                    elements.push_back(math::njVec4f{ c1, c2, c3, c4 });
                } else if (info.type == Type::Vec2 &&
                           info.component_type == ComponentType::Float) {
                    float c1, c2;
                    std::memcpy(&c1, bytes.data() + current_offset, sizeof(float));
                    std::memcpy(&c2, bytes.data() + current_offset + sizeof(float), sizeof(float));
                    elements.push_back(math::njVec2f{ c1, c2 });
                } else if (info.type == Type::Scalar &&
                           info.component_type == ComponentType::UnsignedInt) {
                    uint32_t c1;
                    std::memcpy(&c1, bytes.data() + current_offset, sizeof(uint32_t));
                    elements.push_back(c1);
                } else if (info.type == Type::Scalar &&
                           info.component_type == ComponentType::UnsignedShort) {
                    uint16_t c1;
                    std::memcpy(&c1, bytes.data() + current_offset, sizeof(uint16_t));
                    elements.push_back(c1);
                } else if (info.type == Type::Vec4 &&
                           info.component_type == ComponentType::UnsignedShort) {
                    uint16_t c1, c2, c3, c4;
                    std::memcpy(&c1, bytes.data() + current_offset, sizeof(uint16_t));
                    std::memcpy(&c2, bytes.data() + current_offset + sizeof(uint16_t), sizeof(uint16_t));
                    std::memcpy(&c3, bytes.data() + current_offset + 2 * sizeof(uint16_t), sizeof(uint16_t));
                    std::memcpy(&c4, bytes.data() + current_offset + 3 * sizeof(uint16_t), sizeof(uint16_t));
                    elements.push_back(math::njVec4<uint16_t>{ c1, c2, c3, c4 });
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

    std::vector<uint32_t> Accessor::get_scalar_u32() const {
        std::vector<uint32_t> elements{};
        for (const auto& element : elements_) {
            elements.push_back(std::get<uint32_t>(element));
        }

        return elements;
    }

}  // namespace njin::gltf