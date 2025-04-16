#pragma once
#include <cstdint>

namespace njin::ecs {
    class njComponentMapInterface {
        public:
        virtual ~njComponentMapInterface() = default;

        virtual void remove(uint32_t entity) = 0;
    };
}  // namespace njin::ecs