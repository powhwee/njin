#pragma once
#include <variant>

#include <math/njMat4.h>
#include <math/njVec2.h>
#include <math/njVec3.h>
#include <math/njVec4.h>

#include "util/BufferView.h"

namespace njin::gltf {

    enum class Type {
        Scalar,
        Vec2,
        Vec3,
        Vec4,
        Mat2,
        Mat3,
        Mat4
    };

    // accessor.componentType
    enum class ComponentType : int {
        SignedByte = 5120,
        UnsignedByte = 5121,
        SignedShort = 5122,
        UnsignedShort = 5123,
        UnsignedInt = 5125,
        Float = 5126
    };

    /**
     * Wrapper over a glTF accessor
     */
    class Accessor {
        public:
        /**
         * Bundled information that an accessor needs to read the right values
         * from the bufferview it references
         */
        struct AccessorCreateInfo {
            Type type{ Type::Scalar };
            ComponentType component_type{ ComponentType::SignedByte };
            BufferView buffer_view{};
            uint32_t byte_offset{ 0 };
            uint32_t count{ 0 };
        };

        Accessor(const AccessorCreateInfo& info);

        /**
         * Retrieve all elements the accessor references as an array of njVec2f
         * @return Array of njVec2f elements
         * @note User is responsible for ensuring the type and component type
         * of this accessor are "VEC2" and "FLOAT"
         */
        std::vector<math::njVec2f> get_vec2f() const;

        /**
         * Retrieve all elements the accessor references as an array of njVec2f
         * @return Array of njVec2f elements
         * @note User is responsible for ensuring the type and component type
         * of this accessor are "VEC3" and "FLOAT"
         */
        std::vector<math::njVec3f> get_vec3f() const;

        /**
         * Retrieve all elements the accessor references as an array of njVec4f
         * @return Array of njVec4f elements
         * @note User is responsible for ensuring the type and component type
         * of this accessor are "VEC4" and "FLOAT"
         */
        std::vector<math::njVec4f> get_vec4f() const;

        /**
         * Retrieve all elements the accessor references as an array of njVec3<uint16_t>
         * @return Array of njVec3 elements
         * @note User is responsible for ensuring the type and component type
         * of this accessor are "VEC3" and "UNSIGNED_SHORT"
         */
        std::vector<math::njVec4<uint16_t>> get_vec4ushort() const;

        /**
         * Retrieve all elements the accessor references as an array of uint32_t
         * This is most commonly used to get the result of the indices accessor
         * @return Array of scalars
         * @note User is responsible for ensuring the type and component type
         * of this accessor are "SCALAR" and "UNSIGNED_SHORT"
         */
        std::vector<uint16_t> get_scalar() const;

        private:
        using Element = std::variant<uint16_t,
                                     math::njVec2f,
                                     math::njVec3f,
                                     math::njVec4f,
                                     math::njVec4<uint16_t>>;
        const std::vector<Element> elements_{};
    };

}  // namespace njin::gltf
