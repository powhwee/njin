# njin #

## Features ##

njin is a custom ECS-based game engine / framework

### Framework ###

* Easy, compile-time configuration of components.
    * Adding new data components to the ECS is as easy as adding a new POD
      struct to ```Components.h```
    * No macro or naming conventions enforced for components
    * No registration of custom components needed with the entity manager
        * The entity manager operates on pure templates
* Compile-time archetype classification in the entity manager
    * Any POD struct can be passed as a template argument to the entity manager
    * Archetype signatures are created at compile-time for each unique
      assortment of components housed under a single entity
        * ```typeid``` is used to generate a unique bit signature for an
          archetype.
          Since all components are non-polymorphic there is no runtime overhead
    * Fast archetype signature resolution for entity retrieval / modification
        * Heavy use of type packs with ```std::tuple``` along with pack
          expansion via ```std::apply``` to calculate
          bit signatures at compile-time.
    * Flexible archetype querying
        * Multiple type packs per query allows for ```Include/Exclude``` queries
            * Example query for entities that doesn't have physics, but does
              have a mesh and some transform:
              ```entity_manager.get_views<Include<MeshComponent, TransformComponent>, Exclude<PhysicsComponent>>()```
        * Scoped access
            * ```entity_manager.get_views()``` calls only return access to the
              requested components of an entity

### Engine ###

* Physics System
    * BVH hierarchy
        * Median partitioning - each split tries to keep the number of bounding
          boxes in each half equal
        * Efficient AABB resizing with Arvo90
        * Tri-axis SAT collision testing
    * Simple depenetration
        * Axis-aligned movement along axis of minimum penetration

* Renderer
    * Out-of-box intergration with ECS
        * ```njCameraComponent``` houses configuration for a camera entity
        * View/projection matrices tailored for use with Vulkan
            * No GLM dependency. View/projection calculation algorithm assumes
              Vulkan defaults,
              that is, top-left zero-origin viewport, and a [0, 1] depth space.
    * Custom math
        * Custom vector and matrix representations
        * Shader vertex layout / binding is explicit in ```config.h```, no
          padding surprises
            * Easily modifiable to work with more attributes - simply add a new
              struct in ```config.h```
    * Automatic vertex buffer / descriptor set (layout) / render pass /
      subpass / pipeline creation
        * Based on intuitive configurations in ```config.h```
        * Allows for assignment of names to each resource
        * Calculates buffer sizes, allocates images etc. under the hood
        * When you want to update resources, (copy new data into vertex buffers,
          update the backing data for a particular
          descriptor set, etc), you can refer to them by name to update buffers.
            * Example:
              ```descriptor_sets.write_descriptor_data("mvp", "model", matrices)```
              to update model matrices,
              specifically the "model" binding in the "mvp" descriptor set
            * Id lookup possible for optimization in tight loops
    * Single source of truth in ```config.h``` reduces hard-to-find bugs arising
      from
      consistency issues when setting up Vulkan renderers manually
        * For example, the list of ```VkAttachmentReference``` not matching the
          ```VkAttachmentDescription```
          list when creating the render pass
        * Avoided with named references to resources
    * Windowing system provided by SDL
* Input
    * ```njInputComponent``` wraps SDL to provide an input system
* Asset management
    * Templated ```njRegistryClass``` for keyed access to any resource
    * ```.glb``` file reader for loading in meshes
        * Meshes to load can be configured in ```main.meshes```
        * Mesh data is serialized into ```njMesh``` and retrieved by name via
          ```njRegistry<njMesh>```
            * Position, normal, tangent, uvs, vertex color

## Examples ##

Example of adding an entity with two components to the entity manager

In ```Components.h``` (can be anywhere else):

```
struct MovementComponent {
    float x;
    float y;
    float z;
    float velocity;
}
```

struct HealthComponent {
int max_health;
int current_health;
}

In ```main.cpp```

```
njEntityManager entity_manager{};
EntityId id{entity_manager.add_entity()}; // entity 0

// Initialise component values
MovementComponent movement{
    .x = 0.f,
    .y = 0.f,
    .z = 0.f,
    .velocity = 10.f
};

HealthComponent health {
    .max_health = 100;
    .current_health = 37;
};

entity_manager.add_component(id, movement);
entity_manager.add_component(id, health);
```

Querying for the entity

```
auto nothing_views{entity_manager.get_views<Include<MovementComponent>, Exclude<HealthComponent>>()}; // nothing

auto views{entity_manager.get_views<MovementComponent, HealthComponent>()}; // entity 0

auto movement{std::get<MovementComponent*>(views)};
movement->velocity = 20.f; // entity 0 is now moving at 20 units/s
```

## Setup ##

```
git clone git@github.com:jinruishi/njin.git folder-name
cd folder-name
cmake -S . -B build && cmake --build build
```
