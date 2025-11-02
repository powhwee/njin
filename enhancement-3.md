# Refactoring for Enhancement #3: Multiple Primitives and Meshes

This document details the refactoring work done to implement the third enhancement as described in `gltf-enhancements.md`: support for multiple meshes and primitives in the glTF loader.

## 1. Goal

To refactor the glTF loading pipeline to support files containing multiple meshes and primitives, as outlined in `gltf-enhancements.md`. This moves the engine from loading only the first primitive of the first mesh (`meshes[0].primitives[0]`) to correctly loading all geometry from a `.glb` file.

## 2. Architectural Decision: Self-Contained Loader

During implementation, a key architectural decision was made regarding the role of the `MeshBuilder` class. The chosen path was to make the `gltf::GLTFAsset` loader fully self-contained, responsible for both parsing the file and constructing the final, engine-ready `core::njMesh` objects. This decision was made for the following reasons:

*   **Simplicity:** It provides a more direct and understandable data pipeline from file to engine.
*   **Encapsulation:** The logic for interpreting a specific file format (`.glb`) is entirely contained within its corresponding loader.

This approach made the `MeshBuilder` class redundant, leading to its removal from the project.

## 3. Summary of Changes

This was a significant refactoring of the asset loading pipeline but did not change the engine's core architecture (ECS, Renderer). The primary changes were localized to the `gltf::GLTFAsset` loader and the `core::njMesh` and `core::njPrimitive` data structures.

### 3.1. `core::njPrimitive` Refactoring

The `core::njPrimitive` class was fundamentally changed to correctly represent a glTF primitive.

*   **Before:** The class represented a single triangle (`std::array<njVertex, 3>`).
*   **After:** The class was redefined to represent a proper geometric primitive. It now holds a `std::vector<core::njVertex> vertices_` and a `std::vector<uint32_t> indices_`.
*   **Files Changed:**
    *   `njin/core/include/core/njPrimitive.h`
    *   `njin/core/src/njPrimitive.cpp`

### 3.2. `core::njMesh` Update

Minor changes were made to `core::njMesh` to support named meshes and to correct its behavior with the new `njPrimitive` definition.

*   A public `std::string name;` member was added to store the name of the mesh as defined in the glTF file.
*   The constructor was updated to accept the mesh name.
*   The `get_vertex_count()` and `get_size()` methods were corrected to properly iterate over the new primitive structure.
*   **Files Changed:**
    *   `njin/core/include/core/njMesh.h`
    *   `njin/core/src/njMesh.cpp`

### 3.3. `util::Accessor` Enhancement

To support 32-bit indices (required by the glTF specification), the `Accessor` utility was updated.

*   Added a `get_scalar_u32()` method to read `UNSIGNED_INT` index buffers.
*   The internal `std::variant` was updated to hold `uint32_t`.
*   **Files Changed:**
    *   `njin/util/include/util/Accessor.h`
    *   `njin/util/src/Accessor.cpp`

### 3.4. `gltf::GLTFAsset` Overhaul

The `GLTFAsset` class was completely rewritten to be a self-contained loader.

*   **Before:** The class parsed a single primitive and stored its raw attribute arrays (positions, normals, etc.) locally.
*   **After:** The class is now a pure loader. It parses the entire `.glb` file and constructs a `std::vector<core::njMesh>`.
    *   The constructor now contains nested loops to iterate through every `mesh` in the glTF file and every `primitive` within each mesh.
    *   For each glTF primitive, it constructs a `std::vector<core::njVertex>` by combining the various attribute arrays (positions, normals, tangents, etc.).
    *   It then creates a `core::njPrimitive` from these vertices and the corresponding index buffer.
    *   These primitives are grouped into a `core::njMesh` object, which is added to an internal vector.
    *   All local geometry attribute members were removed, replaced by a `get_meshes()` method that returns the final vector of engine-ready meshes.
*   **Files Changed:**
    *   `njin/util/include/util/GLTFAsset.h`
    *   `njin/util/src/GLTFAsset.cpp`

### 3.5. `MeshBuilder` Removal

As a direct consequence of the new loader design, `MeshBuilder` became obsolete.

*   The `load_meshes` function in `njin/core/src/loader.cpp` was updated to use the new `GLTFAsset::get_meshes()` method, removing all calls to `MeshBuilder`.
*   The `MeshBuilder` source and test files were removed from the build system.
*   **Files Changed:**
    *   `njin/core/src/loader.cpp`
    *   `njin/core/CMakeLists.txt`
*   **Files to be Deleted:**
    *   `njin/core/include/core/MeshBuilder.h`
    *   `njin/core/src/MeshBuilder.cpp`
    *   `njin/core/test/MeshBuilder_test.cpp`

## 4. Post-Enhancement Debugging: Fixing Indexed Drawing

After the refactoring for Enhancement #3, a visual bug was identified where the rendered cube appeared with faint, dotted, and partially invisible lines instead of a solid white wireframe. This section documents the debugging process and the subsequent fix.

### 4.1. Investigation

The initial investigation pointed towards a discrepancy between the data being sent to the GPU and how it was being rendered.

1.  **Shader Analysis:** The vertex and fragment shaders (`shader.vert`, `shader.frag`) were examined. They were found to be correct for rendering a simple, solid color, ruling out any issues with the shader logic itself.

2.  **Drawing Command Analysis:** The core of the issue was discovered in the rendering commands. A search for `vkCmdDraw` and `vkCmdDrawIndexed` revealed that the renderer was still using `vkCmdDraw`.

The refactoring in Enhancement #3 had correctly updated the `core::njPrimitive` class to hold both vertices and indices, but the rendering pipeline was never updated to use this new index data. It was still drawing vertices directly from the vertex buffer in a non-indexed manner, leading to the incorrect visual output.

### 4.2. Implementing Indexed Drawing

To fix this, a significant change was made to the Vulkan rendering backend to switch from `vkCmdDraw` to `vkCmdDrawIndexed`. This was a multi-step process involving changes across the rendering pipeline.

#### 4.2.1. Index Buffer Management

A new class, `IndexBuffers`, was created to manage `VkBuffer` resources specifically for index data. This mirrored the existing `VertexBuffers` implementation for consistency.

*   **New Files:**
    *   `njin/vulkan/include/vulkan/IndexBuffers.h`
    *   `njin/vulkan/src/IndexBuffers.cpp`
*   **Configuration:**
    *   A new `IndexBufferInfo` struct was added to `njin/vulkan/include/vulkan/config_classes.h`.
    *   The main `RenderResourceInfos` struct in `njin/vulkan/include/vulkan/RenderResources.h` was updated to include a vector of `IndexBufferInfo`.
    *   The `RenderResources` class was updated to create and manage the `IndexBuffers`.
    *   The `CMakeLists.txt` for the `vulkan` library was updated to include the new `IndexBuffers.cpp` source file.

#### 4.2.2. Populating Render Information

The `RenderInfos` class, responsible for preparing data for the renderer, was heavily modified.

*   **File Changed:** `njin/vulkan/src/RenderInfos.cpp`
*   **Changes:**
    *   The `write_data` function was overhauled to create and populate a consolidated index buffer (`main_indices`).
    *   As it iterates through each `core::njPrimitive`, it now adds the primitive's vertices to the vertex buffer and its indices to the new index buffer.
    *   Crucially, the indices are re-mapped to be global by adding the current vertex offset. This allows all primitives to be stored in a single vertex/index buffer pair.
    *   A `MeshRenderInfo` is now generated for *each primitive*, containing the necessary information for an indexed draw call: `index_count`, `first_index`, and `vertex_offset`.

#### 4.2.3. Updating the Draw Call

The final step was to change the drawing command itself in the `SubpassModule`.

*   **File Changed:** `njin/vulkan/src/SubpassModule.cpp`
*   **Changes:**
    *   The `record` function now binds the newly created index buffer using `vkCmdBindIndexBuffer`.
    *   The `vkCmdDraw` call was replaced with `vkCmdDrawIndexed`.
    *   The parameters for the new draw call are supplied from the `MeshRenderInfo` populated in the previous step.

#### 4.2.4. Renaming `mesh_offset`

During the refactoring, the `mesh_offset` field in `MeshRenderInfo` was renamed to `vertex_offset`. This was done for two main reasons:

1.  **Clarity:** To make it explicit that the field represents an offset into the vertex buffer.
2.  **Consistency:** To align with the `vertexOffset` parameter in the `vkCmdDrawIndexed` function, even though in the current implementation this value is `0` due to the use of global indices.

This change improves code readability and consistency with the underlying Vulkan API. The comment for `index_count` was also corrected to reflect that it stores the number of indices, not vertices.