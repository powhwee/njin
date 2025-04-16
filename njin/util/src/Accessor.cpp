#include "util/Accessor.h"

namespace njin::gltf {

    Accessor::Accessor(const AccessorCreateInfo& info) :
        elements_{ [&]() -> std::vector<Element> {
            std::vector<Element> elements{};

            uint32_t current_offset{ info.byte_offset };

            std::vector<std::byte> bytes{ info.buffer_view.get() };

            uint32_t current_element{ 0 };
            while (current_element < info.count) {
                if (info.type == Type::Vec3 &&
                    info.component_type == ComponentType::Float) {
                    float component_1{};
                    std::memcpy(&component_1,
                                bytes.data() + current_offset,
                                sizeof(float));
                    current_offset += sizeof(float);

                    float component_2{};
                    std::memcpy(&component_2,
                                bytes.data() + current_offset,
                                sizeof(float));
                    current_offset += sizeof(float);

                    float component_3{};
                    std::memcpy(&component_3,
                                bytes.data() + current_offset,
                                sizeof(float));
                    current_offset += sizeof(float);

                    math::njVec3f element{ component_1,
                                           component_2,
                                           component_3 };
                    elements.push_back(element);
                }

                else if (info.type == Type::Vec4 &&
                         info.component_type == ComponentType::Float) {
                    float component_1{};
                    std::memcpy(&component_1,
                                bytes.data() + current_offset,
                                sizeof(float));
                    current_offset += sizeof(float);

                    float component_2{};
                    std::memcpy(&component_2,
                                bytes.data() + current_offset,
                                sizeof(float));
                    current_offset += sizeof(float);

                    float component_3{};
                    std::memcpy(&component_3,
                                bytes.data() + current_offset,
                                sizeof(float));
                    current_offset += sizeof(float);

                    float component_4{};
                    std::memcpy(&component_4,
                                bytes.data() + current_offset,
                                sizeof(float));
                    current_offset += sizeof(float);

                    math::njVec4<float> element{ component_1,
                                                 component_2,
                                                 component_3,
                                                 component_4 };
                    elements.push_back(element);
                }

                else if (info.type == Type::Vec2 &&
                         info.component_type == ComponentType::Float) {
                    float component_1{};
                    std::memcpy(&component_1,
                                bytes.data() + current_offset,
                                sizeof(float));
                    current_offset += sizeof(float);

                    float component_2{};
                    std::memcpy(&component_2,
                                bytes.data() + current_offset,
                                sizeof(float));
                    current_offset += sizeof(float);

                    math::njVec2f element{ component_1, component_2 };
                    elements.push_back(element);
                }

                // indices
                else if (info.type == Type::Scalar &&
                         info.component_type == ComponentType::UnsignedShort) {
                    uint16_t component{};
                    std::memcpy(&component,
                                bytes.data() + current_offset,
                                sizeof(uint16_t));
                    current_offset += sizeof(uint16_t);

                    elements.push_back(component);
                }
                // colors
                else if (info.type == Type::Vec4 &&
                         info.component_type == ComponentType::UnsignedShort) {
                    uint16_t component_0{};
                    std::memcpy(&component_0,
                                bytes.data() + current_offset,
                                sizeof(uint16_t));
                    current_offset += sizeof(uint16_t);

                    uint16_t component_1{};
                    std::memcpy(&component_1,
                                bytes.data() + current_offset,
                                sizeof(uint16_t));
                    current_offset += sizeof(uint16_t);

                    uint16_t component_2{};
                    std::memcpy(&component_2,
                                bytes.data() + current_offset,
                                sizeof(uint16_t));
                    current_offset += sizeof(uint16_t);

                    uint16_t component_3{};
                    std::memcpy(&component_3,
                                bytes.data() + current_offset,
                                sizeof(uint16_t));
                    current_offset += sizeof(uint16_t);

                    math::njVec4<uint16_t> element{ component_0,
                                                    component_1,
                                                    component_2,
                                                    component_3 };
                    elements.push_back(element);
                }
                ++current_element;
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

}  // namespace njin::gltf