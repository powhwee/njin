#pragma once
#include <iostream>

namespace njin::ecs {

    template<typename Component>
    void njEntityManager::add_component(EntityId entity, Component component) {
        ComponentType component_type{ typeid(Component) };
        if (type_to_components_.contains(component_type)) {
            njComponentMap<Component>* map{
                static_cast<njComponentMap<Component>*>(type_to_components_
                                                        .at(component_type)
                                                        .get())
            };

            map->insert(entity, component);
        } else {  // new component type
            // record the new type's bit signature
            type_to_signature_.insert({ component_type, current_signature_ });
            signature_to_type_.insert({ current_signature_, component_type });
            current_signature_ <<= 1;

            // make a new map that stores this component type
            njComponentMap<Component> new_map{};
            new_map.insert(entity, component);
            type_to_components_
            .insert({ component_type,
                      std::make_unique<njComponentMap<Component>>(new_map) });
        }

        ComponentSignature new_signature{
            calculate_new_signature(entity, component_type)
        };
        update_entity_signature(entity, new_signature);
    }

    template<typename... Component>
    requires(ComponentTypes<Component...>)
    std::vector<View<Component...>> njEntityManager::get_views() const {
        // all the relevant component maps
        std::tuple<njComponentMap<Component>*...> maps{
            get_component_map<Component>()...
        };

        // if a component that does not yet exist in the entity manager
        // was used in a query then we know there will be zero matches
        bool should_terminate{ std::apply(
        [](njComponentMap<Component>*... m) { return ((m == nullptr) || ...); },
        maps) };
        // check and early return if necessary
        if (should_terminate) {
            return {};
        }

        // find the entities that fit the requested signature
        const ComponentSignature signature{
            calculate_signature<Component...>()
        };

        const std::set<EntityId> entities{
            get_entities_by_signature(signature)
        };

        std::vector<View<Component...>> views{};
        for (EntityId entity : entities) {
            // get the view of a single entity
            View<Component...> view{ std::apply(
            [entity](njComponentMap<Component>*... map) -> View<Component...> {
                std::tuple<Component*...> components{
                    std::make_tuple(map->get(entity)...)
                };
                return { entity, components };
            },
            maps) };
            views.push_back(view);
        }

        return views;
    }

    inline EntityId njEntityManager::add_entity(const EntityName& name) {
        // record the name associated with this entity
        name_to_id_.insert({ name, current_ });
        const EntityId this_id{ current_ };
        ++current_;

        update_entity_signature(this_id, 0b0);

        return this_id;
    }

    template<typename Component>
    njComponentMap<Component>* njEntityManager::get_component_map() const {
        ComponentType type_id{ typeid(Component) };
        if (type_to_components_.contains(type_id)) {
            return static_cast<njComponentMap<Component>*>(type_to_components_
                                                           .at(type_id)
                                                           .get());
        }
        return nullptr;
    }

    inline void
    njEntityManager::update_entity_signature(EntityId entity,
                                             ComponentSignature signature) {
        // this entity is new, so it doesn't have a previous signature
        if (!id_to_signature_.contains(entity)) {
            id_to_signature_.insert({ entity, signature });
            if (!signature_to_ids_.contains(signature)) {
                signature_to_ids_.insert({ signature, { entity } });
            } else {
                std::set<EntityId>& new_bucket{
                    signature_to_ids_.at(signature)
                };
                new_bucket.insert(entity);
            }
            return;
        }
        ComponentSignature old_signature{ id_to_signature_.at(entity) };
        id_to_signature_.at(entity) = signature;

        // remove from old signature bucket
        std::set<EntityId>& old_bucket{ signature_to_ids_.at(old_signature) };
        old_bucket.erase(entity);

        // add to new signature bucket
        // need to check if that bucket actually exists yet
        if (!signature_to_ids_.contains(signature)) {
            signature_to_ids_.insert({ signature, { entity } });
        } else {
            std::set<EntityId>& new_bucket{ signature_to_ids_.at(signature) };
            new_bucket.insert(entity);
        }
    }

    inline njEntityManager::ComponentSignature
    njEntityManager::calculate_new_signature(EntityId entity,
                                             ComponentType new_component_type) {
        ComponentSignature old_signature{ id_to_signature_.at(entity) };
        ComponentSignature component_signature{
            type_to_signature_.at(new_component_type)
        };

        return old_signature | component_signature;
    }

    inline std::set<EntityId>
    njEntityManager::get_entities_by_signature(ComponentSignature requirement)
    const {
        std::set<EntityId> result{};
        // special case where the requirement is entities with no component
        // at all.
        if (requirement.none()) {
            return signature_to_ids_.at(0);
        }

        for (auto [signature, entities] : signature_to_ids_) {
            if ((requirement & signature) == requirement) {
                result.insert(entities.begin(), entities.end());
            }
        }
        return result;
    }

    template<typename... Component>
    njEntityManager::ComponentSignature
    njEntityManager::calculate_signature() const {
        std::vector<ComponentSignature> signatures{
            get_signature<Component>()...
        };

        ComponentSignature result{};
        for (const auto signature : signatures) {
            result |= signature;
        }

        return result;
    }

    template<typename Component>
    njEntityManager::ComponentSignature njEntityManager::get_signature() const {
        if (!type_to_signature_.contains(typeid(Component))) {
            return { 0b0 };
        }
        return type_to_signature_.at(typeid(Component));
    }

    template<typename... Component>
    void njEntityManager::remove_components(EntityId entity) {
        // remove the components from their maps
        std::tuple<njComponentMap<Component>*...> maps{
            get_component_map<Component>()...
        };

        std::apply(
        [entity](njComponentMap<Component>*... m) { (m->remove(entity), ...); },
        maps);

        // update the signature
        ComponentSignature old{ id_to_signature_.at(entity) };
        ComponentSignature remove{ calculate_signature<Component>()... };
        ComponentSignature new_signature{ (old ^ remove) & old };

        update_entity_signature(entity, new_signature);
    }

    inline void njEntityManager::remove_entity(EntityId entity) {
        // remove from signature maps
        ComponentSignature signature{ id_to_signature_.at(entity) };
        id_to_signature_.erase(entity);
        std::set<EntityId>& bucket{ signature_to_ids_.at(signature) };
        bucket.erase(entity);

        /** remove entity's components from all component maps */
        std::vector<ComponentType> types{};
        for (int i{ 0 }; i < SIGNATURE_LENGTH; ++i) {
            if (signature.test(0)) {
                ComponentSignature current{ 0b1 };
                current <<= i;
                if (!signature_to_type_.contains(current)) {
                    continue;
                }
                types.push_back(signature_to_type_.at(0b1 << i));
            }
        }

        for (ComponentType type : types) {
            njComponentMapInterface* map{ type_to_components_.at(type).get() };
            map->remove(entity);
        }
    }

    template<typename... Component>
    View<Component...> njEntityManager::get_view(EntityId entity) const {
        // all the relevant component maps
        std::tuple<njComponentMap<Component>*...> maps{
            get_component_map<Component>()...
        };

        View<Component...> view{ std::apply(
        [entity](njComponentMap<Component>*... map) -> View<Component...> {
            std::tuple<Component*...> components{
                std::make_tuple(map->get(entity)...)
            };
            return { entity, components };
        },
        maps) };

        return view;
    }

    template<ComponentTypePack Include, ComponentTypePack Exclude>
    std::vector<typename Include::view_type>
    njEntityManager::get_views() const {
        // Include::component_types is a tuple of types which is NOT the same
        // as a parameter pack, so we cannot perform pack expansion!
        // We do some magic to use the tuple of types as a parameter pack below.
        // We use a variadic template lambda to introduce a named pack
        // "Components".
        // Then, we wrap the tuple of types into a struct (std::type_identity)
        // and pass it as a function argument. This tricks the compiler into
        // deducing the pack, thus successfully "re-packing" the tuple of
        // types into a pack.

        // maps is of type std::tuple<njComponentMap<Component>*...>
        // aka a tuple of pointers to component maps, one for each specified
        // component type
        auto maps{ [&]<typename... Components>(
                   std::type_identity<std::tuple<Components...>>) {
            return std::make_tuple(get_component_map<Components>()...);
        }(std::type_identity<typename Include::component_types>{}) };

        ComponentSignature include_signature{
            [&]<typename... Components>(
            std::type_identity<std::tuple<Components...>>) {
                return calculate_signature<Components...>();
            }(std::type_identity<typename Include::component_types>{})
        };

        ComponentSignature exclude_signature{
            [&]<typename... Components>(
            std::type_identity<std::tuple<Components...>>) {
                return calculate_signature<Components...>();
            }(std::type_identity<typename Exclude::component_types>{})
        };

        // set difference
        std::set<EntityId> includes{
            get_entities_by_signature(include_signature)
        };
        std::set<EntityId> excludes{
            get_entities_by_signature(exclude_signature)
        };

        auto in_excludes{ [&excludes](EntityId id) {
            return excludes.contains(id);
        } };
        std::erase_if(includes, in_excludes);

        std::vector<typename Include::view_type> views{};
        for (EntityId entity : includes) {
            // get the view of a single entity
            auto view{ std::apply(
            [entity]<typename... Components>(njComponentMap<Components>*... map)
            -> View<Components...> {
                std::tuple<Components*...> components{
                    std::make_tuple(map->get(entity)...)
                };
                return { entity, components };
            },
            maps) };
            views.push_back(view);
        }

        return views;
    }
}  // namespace njin::ecs
