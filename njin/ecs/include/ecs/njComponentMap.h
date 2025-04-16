#pragma once
#include <cstdint>
#include <unordered_map>

#include "njComponentMapInterface.h"

namespace njin::ecs {
    template<typename Component>
    class njComponentMap : public njComponentMapInterface {
        public:
        using EntityId = uint32_t;
        void remove(EntityId entity) override;

        void insert(EntityId entity, Component component);

        Component* get(EntityId entity);

        private:
        std::unordered_map<EntityId, Component> map_{};
    };
}  // namespace njin::ecs

#include "njComponentMap.tpp"