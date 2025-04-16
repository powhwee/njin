namespace njin::ecs {
    template<typename Component>
    void njComponentMap<Component>::insert(EntityId entity,
                                           Component component) {
        map_.insert({ entity, component });
    }

    template<typename Component>
    Component* njComponentMap<Component>::get(EntityId entity) {
        return &map_.at(entity);
    }

    template<typename Component>
    void njComponentMap<Component>::remove(EntityId entity) {
        map_.erase(entity);
    }

};  // namespace njin::ecs