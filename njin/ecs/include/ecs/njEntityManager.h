#pragma once
#include <bitset>
#include <cstdint>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

#include "ecs/EngineTypes.h"
#include "ecs/njComponentMap.h"
#include "ecs/njComponentMapInterface.h"

#include <any>
#include <set>

namespace njin::ecs {

    template<typename T>
    concept ComponentTypePack = requires {
        // the type of a View of the given Components
        typename T::view_type;

        // tuple of component types
        typename T::component_types;
    };

    template<typename... T>
    concept ComponentTypes = (!ComponentTypePack<T> && ...);

    // A View is a literal "view" into an entity. For example,
    // suppose entity A has 3 components, C0, C1, C2
    // then View<C0, C1> is a tuple of pointers to C0 and C1 of A.
    template<typename... Components>
    using View = std::pair<EntityId, std::tuple<Components*...>>;

    template<typename... Components>
    struct Include {
        using view_type = View<Components...>;
        using component_types = std::tuple<Components...>;
    };

    // identical definition, differs only semantically
    template<typename... Components>
    using Exclude = Include<Components...>;

    class njEntityManager {
        public:
        static constexpr int SIGNATURE_LENGTH = 32;
        using ComponentType = std::type_index;
        using EntityName = std::string;
        using ComponentSignature = std::bitset<SIGNATURE_LENGTH>;

        // the component signature of an entity completely determines the components
        // that entity has
        struct ComponentSignatureHash {
            std::size_t operator()(const ComponentSignature& b) const {
                return b
                .to_ullong();  // Works if N ≤ 64, otherwise use a custom hash
            }
        };

        /**
         * Add a component to an existing entity
         * @tparam Component Type of component
         * @param entity Entity to add component to
         * @param component Component to add
         */
        template<typename Component>
        void add_component(EntityId entity, Component component);

        /**
         * Create a new entity
         * @param name Name to associate with the entity
         * @return EntityId given to the new entity
         */
        EntityId add_entity(const EntityName& name);

        /**
         * Remove an entity and all its components
         * @param entity Entity to remove
         */
        void remove_entity(EntityId entity);

        /**
         * Query for a particular view across all entities
         * @tparam Component Component types to query for
         * @return List of views
         */
        template<typename... Component>
        requires(ComponentTypes<Component...>)
        std::vector<View<Component...>> get_views() const;

        /**
         * Query for a particular view across all entities
         * @tparam Include Component types to include
         * @tparam Exclude Component types to exclude
         * @return List of views
         * @note if the same component type is in both the exclude list
         * and the include list, the overall effect is that entities with
         * that component type are still excluded
         */
        template<ComponentTypePack Include,
                 ComponentTypePack Exclude = Exclude<>>
        std::vector<typename Include::view_type> get_views() const;

        /**
         * Query for a particular view of an entity
         * @tparam Component Component types to query for
         * @return view of the given entity
         */
        template<typename... Component>
        View<Component...> get_view(EntityId entity) const;

        /**
         * Remove component(s) from an entity
         * @tparam Component List of components type(s) to remove
         * @param entity Entity to remove components from
         */
        template<typename... Component>
        void remove_components(EntityId entity);

        private:
        std::unordered_map<ComponentType,
                           std::unique_ptr<njComponentMapInterface>>
        type_to_components_;
        std::unordered_map<EntityName, EntityId> name_to_id_{};
        std::unordered_map<ComponentType, ComponentSignature>
        type_to_signature_{};
        std::unordered_map<ComponentSignature, ComponentType>
        signature_to_type_{};
        std::unordered_map<EntityId, ComponentSignature> id_to_signature_{};

        // all entities with the same component signature are stored
        // in the same bucket
        std::unordered_map<ComponentSignature, std::set<EntityId>>
        signature_to_ids_{};

        EntityId current_{ 0 };

        // current available signature for a new component
        ComponentSignature current_signature_{ 0b1 };

        /**
         * Get the map that contains a certain component type
         * @tparam Component Component type
         * @return Component map. If the component does not have a map
         * then it returns nullptr
         */
        template<typename Component>
        njComponentMap<Component>* get_component_map() const;

        /**
         * Update the respective maps when an entity has a new component signature
         * @param entity EntityId of the entity
         * @param signature New component signature of the entity
         */
        void update_entity_signature(EntityId entity,
                                     ComponentSignature signature);

        /**
       * Recalculate the signature of a given entity
       * @param entity Entity to calculate a new signature for
       * @param new_component_type The type of the new component added to the entity
       * @return
       */
        ComponentSignature
        calculate_new_signature(EntityId entity,
                                ComponentType new_component_type);

        /**
         * Calculate the signature of a given set of components
         * @tparam Component Component types
         * @return Signature
         */
        template<typename... Component>
        ComponentSignature calculate_signature() const;

        /**
         * Get the signature of a certain component type
         * @tparam Component Component type
         * @return Signature of component. If the component does not exist
         * in the entity manager then the zero signature is returned
         */
        template<typename Component>
        ComponentSignature get_signature() const;

        /**
         * Get all the Entities whose component signatures satisfy the
         * given signature. An Entity has a satisfactory component signature
         * if all the bits set in the requirement signature are also set
         * in the component signature.
         * That is, if Entity A has components C0, C1, C2, the requirement signature
         * of {C0, C1} is satisfied, and so is {C0}, and so forth.
         * @param requirement Requirement signature, the signature to satisfy
         * @return List of Entities
         */
        std::set<EntityId>
        get_entities_by_signature(ComponentSignature requirement) const;
    };

}  // namespace njin::ecs

#include "ecs/njEntityManager.tpp"