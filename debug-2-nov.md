# Debugging Session: November 2nd

This document summarizes a debugging session focused on resolving issues encountered after attempting to load `toy-rocket.glb` into the `njin` engine.

## Initial Problem: Blank Screen and Runtime Error

**User Action:** Changed `main.meshes` to reference `toy-rocket.glb`.
**Observed Behavior:** Program ran, but nothing was visible on screen. A runtime error occurred.

**Error Log (First Instance):**
```
Validation Warning: [ WARNING-DEBUG-PRINTF ] | MessageID = 0x76589099
vkCreateDevice(): Internal Warning: Adding a VkPhysicalDeviceScalarBlockLayoutFeatures to pNext with scalarBlockLayout set to VK_TRUE
...
Process 98277 stopped
* thread #1, queue = 'com.apple.main-thread', stop reason = EXC_BAD_ACCESS (code=2, address=0x115110000)
    frame #0: 0x000000019bfb63b8 libsystem_platform.dylib`_platform_memmove + 88
    frame #1: 0x0000000100109f84 njin`void njin::vulkan::Buffer::load<njin::vulkan::VERTEX_INPUT_MAIN_DRAW_FORMAT>(this=0x0000000b550f5e28, vec=size=47119) at Buffer.h:45:13
    frame #2: 0x00000001000fb958 njin`void njin::vulkan::VertexBuffers::load_into_buffer<njin::vulkan::VERTEX_INPUT_MAIN_DRAW_FORMAT>(this=0x000000016fdfe328, name="main_draw", data=size=47119) at VertexBuffers.h:22:20
    frame #3: 0x00000001000fa480 njin`njin::vulkan::RenderInfos::write_data(this=0x000000016fdfdba0, mesh_registry=0x000000016fdfe0c8, texture_registry=0x000000016fdfe098, render_resources=0x000000016fdfe168, view_matrix=0x000000016fdfd90c, projection_matrix=0x000000016fdfd8cc) at RenderInfos.cpp:277:45
...
```

**Diagnosis:**
The `EXC_BAD_ACCESS` occurred during a memory copy operation (`_platform_memmove`) when loading vertex data into the "main_draw" vertex buffer. The `vec` parameter had a size of 47119 elements. The most likely cause was that the allocated vertex buffer was too small to hold the data from `toy-rocket.glb`.

**Action Taken:**
Increased `max_vertex_count` to 100,000 and `max_index_count` to 150,000 in `njin/vulkan/include/vulkan/config.h`.

## Second Problem: Zero-Byte Buffer Allocation

**Observed Behavior:** After increasing buffer sizes, the program crashed with a `std::runtime_error: failed to allocate buffer memory`.

**Error Log (Second Instance):**
```
Validation Error: [ VUID-VkBufferCreateInfo-size-00912 ] | MessageID = 0x7c54445e
vkCreateBuffer(): pCreateInfo->size is zero.
...
libc++abi: terminating due to uncaught exception of type std::runtime_error: failed to allocate buffer memory
...
  * frame #0: 0x000000019bf725b0 libsystem_kernel.dylib`__pthread_kill + 8
...
    frame #9: 0x0000000100087020 njin`njin::vulkan::Buffer::Buffer(this=0x000000016fdfd818, device=0x000000016fdfe540, physical_device=0x000000016fdfe558, settings=0x000000016fdfd858) at Buffer.cpp:52:13
...
```

**Diagnosis:**
The `vkCreateBuffer()` and `vkAllocateMemory()` calls were receiving a size of zero. Debug prints confirmed that `buffer_info.vertex_input.vertex_size` was 0 for the `main_draw` vertex buffer, despite `VERTEX_INPUT_INFO_MAIN_DRAW` in `config.h` explicitly setting `vertex_size = 12`. This was diagnosed as a **static initialization order fiasco** or a similar issue with `inline` variables, where `VERTEX_INPUT_INFO_MAIN_DRAW` was not being correctly initialized across translation units.

**Proposed Fix:**
Add a constructor to `VertexInputInfo` in `njin/vulkan/include/vulkan/config_classes.h` to dynamically calculate `vertex_size` based on its `attribute_infos`. Then, update the initialization of `VERTEX_INPUT_INFO_MAIN_DRAW` and `VERTEX_INPUT_INFO_ISO_DRAW` in `njin/vulkan/include/vulkan/config.h` to use this new constructor.

## Third Problem: Compilation Errors (No Matching Constructor / Designated Initializer List)

**Observed Behavior:** After applying the proposed fix, compilation failed.

**Error Log (Third Instance):**
```
/Users/powhweee/coding/njin/njin/vulkan/include/vulkan/config.h:366:5: error: no matching constructor for initialization of 'VertexInputInfo'
  366 |     };
      |     ^
...
/Users/powhweee/coding/njin/njin/vulkan/include/vulkan/config.h:184:28: error: initialization of non-aggregate type 'VertexInputInfo' with a designated initializer list
  184 |     inline VertexInputInfo VERTEX_INPUT_INFO_MAIN_DRAW{
      |                            ^                          ~
...
```

**Diagnosis:**
The `VertexInputInfo` struct, having a user-defined constructor, was no longer an aggregate type. Therefore, it could not be initialized using aggregate initialization syntax (`{...}` or designated initializers `.name = ...`). The fix required changing the initialization of `VERTEX_INPUT_INFO_MAIN_DRAW` and `VERTEX_INPUT_INFO_ISO_DRAW` to explicitly call the constructor using parentheses `(...)`.

**Action Taken:**
Modified `njin/vulkan/include/vulkan/config.h` to change the initialization of `VERTEX_INPUT_INFO_MAIN_DRAW` and `VERTEX_INPUT_INFO_ISO_DRAW` from designated initializer lists to explicit constructor calls.

## Fourth Problem: Blank Screen (Again) and Mesh Not Found

**Observed Behavior:** Program ran, buffer sizes were correct, but nothing was visible on screen. A runtime error occurred: `Item with name 'cube' does not exist.`

**Error Log (Fourth Instance):**
```
Creating vertex buffer: main_draw, vertex_size: 12, max_vertex_count: 100000, calculated size: 1200000
...
Item with name 'cube' does not exist.
Process 42313 stopped
* thread #1, queue = 'com.apple.main-thread', stop reason = EXC_BAD_ACCESS (code=1, address=0x18)
    frame #0: 0x00000001003ad944 njin`njin::ecs::njRenderSystem::update(this=0x00000009e531f6a0, entity_manager=0x000000016fdfdf80) at njRenderSystem.cpp:228:39
   228 |             math::njVec3f local_min = mesh.min_bounds;
```

**Diagnosis:**
The `mesh_registry_->get("cube")` call in `njRenderSystem` was failing because the actual keys in the `mesh_registry` were prefixed (e.g., `"cube-Object_0"`, `"cube-Cube.001"`), as confirmed by `njin/core/src/loader.cpp` and debug output from `GLTFAsset.cpp`. The `njMeshComponent` was storing the high-level name ("cube"), not the full registry key.

**Proposed Fix:**
1.  **Modify `njin/ecs/include/ecs/Components.h`:** Rename `std::string mesh;` to `std::string registry_key;` in `njMeshComponent`.
2.  **Modify `njin/ecs/njObjectArchetype.h` and `njin/ecs/src/njObjectArchetype.cpp`:** Update `njObjectArchetypeCreateInfo` and `njObjectArchetype::make_archetype` to use `registry_key`.
3.  **Modify `main.cpp`:** Set `registry_key` to `"cube-Object_0"` for the cube object.
4.  **Modify `njin/ecs/njPlayerArchetype.h` and `njin/ecs/src/njPlayerArchetype.cpp`:** Update `njPlayerArchetypeCreateInfo` and `njPlayerArchetype::make_archetype` to use `registry_key`.
5.  **Modify `njin/ecs/src/njRenderSystem.cpp`:** Use `mesh_component->registry_key` directly for the `mesh_registry_->get()` call.

## Fifth Problem: Compilation Errors in `njRenderSystem.cpp` (Function Definitions, Redefinitions, Missing Brace)

**Observed Behavior:** After applying the fix for the mesh lookup, compilation failed with multiple errors.

**Error Log (Fifth Instance):**
```
/Users/powhweee/coding/njin/njin/ecs/src/njRenderSystem.cpp:217:53: error: function definition is not allowed here
  217 |                                        EntityId id) {
      |                                                     ^
/Users/powhweee/coding/njin/njin/ecs/src/njRenderSystem.cpp:319:29: error: redefinition of 'njRenderSystem'
  319 |             njRenderSystem::njRenderSystem(core::RenderBuffer& buffer, core::njRegistry<core::njMesh>& mesh_registry) :
      |                             ^
/Users/powhweee/coding/njin/njin/ecs/src/njRenderSystem.cpp:585:38: error: expected '}'
  585 |             }  // namespace njin::ecs
      |                                      ^
```

**Diagnosis:**
My previous large `replace` operation in `njin/ecs/src/njRenderSystem.cpp` inadvertently caused several issues:
*   Helper functions (`calculate_model_matrix`, `make_wireframe`, `transform_point`) were moved *inside* `njRenderSystem::update`, leading to "function definition not allowed here" errors.
*   The `njRenderSystem` constructor and `update` method were redefined within the same file, causing "redefinition" errors.
*   The closing brace for the `namespace njin::ecs` block was removed, leading to an "expected '}'" error.

**Current Goal:**
Fix the compilation errors in `njin/ecs/src/njRenderSystem.cpp` by:
1.  Correctly placing helper functions in the anonymous namespace.
2.  Ensuring `njRenderSystem` constructor and `update` method are defined only once.
3.  Adding the missing closing brace for the `njin::ecs` namespace.
4.  Applying the necessary fixes for pointer dereferencing, vector operations, camera component access, and transform assignment within the `update` method.
