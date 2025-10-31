# Enhancement #3: Multiple Mesh and Primitive Loading

## Goal

To refactor the glTF loading pipeline to support files containing multiple meshes and primitives, as outlined in `gltf-enhancements.md`. This moves the engine from loading only the first primitive of the first mesh (`meshes[0].primitives[0]`) to correctly loading all geometry from a `.glb` file.

## Architectural Impact

This was a significant refactoring of the asset loading pipeline but did not change the engine's core architecture (ECS, Renderer). The primary changes were localized to the `gltf::GLTFAsset` loader and the `core::njMesh` and `core::njPrimitive` data structures.

## File-by-File Change Breakdown

### 1. `njin/core/include/core/njPrimitive.h`

*   **Before:** The `njPrimitive` class represented a single triangle, holding a `std::array<njVertex, 3>`.
*   **After:** The class was redefined to represent a proper glTF primitive (a batch of geometry).
    *   The `std::array` was replaced with `std::vector<njVertex> vertices_{};`.
    *   A `std::vector<uint32_t> indices_{};` was added to hold the index buffer for the primitive.
    *   A `uint32_t material_index_{};` was added to store the material associated with this primitive.
    *   The constructor was updated to accept these new members.

### 2. `njin/core/src/njPrimitive.cpp`

*   **Before:** Implemented a constructor that accepted a `std::array<njVertex, 3>`.
*   **After:** The implementation was updated to match the new header. The constructor now accepts and initializes the `vertices_`, `indices_`, and `material_index_` members.

### 3. `njin/core/include/core/njMesh.h`

*   **Before:** The class held a `std::vector<njPrimitive>` but had no identifying name.
*   **After:**
    *   A public `std::string name;` member was added to store the name of the mesh as defined in the glTF file.
    *   The constructor was updated to accept the new `name` string.

### 4. `njin/core/src/njMesh.cpp`

*   **Before:** The constructor only accepted primitives. The `get_size()` and `get_vertex_count()` methods contained logic based on the assumption that each primitive was a single triangle.
*   **After:**
    *   The constructor was updated to accept and initialize the new `name` member.
    *   The `get_size()` and `get_vertex_count()` methods were rewritten to correctly loop through the new `njPrimitive` structures and sum the sizes of their `vertices_` vectors, making the calculations accurate for primitives of any size.

### 5. `njin/util/include/util/GLTFAsset.h`

*   **Before:** The loader's header file declared many private member vectors for individual vertex attributes (`position_attributes_`, `normal_attributes_`, etc.) and exposed them via multiple public getter methods (`get_position_attributes()`, etc.).
*   **After:**
    *   All private member vectors for geometry attributes were removed.
    *   They were replaced by a single `std::vector<core::njMesh> meshes_{};`.
    *   All public `get_*` methods were removed and replaced with a single method: `std::vector<core::njMesh> get_meshes() const;`.

### 6. `njin/util/src/GLTFAsset.cpp`

*   **Before:** The constructor contained hardcoded logic that accessed `meshes[0]` and `primitives[0]`. It parsed the geometry data from this single primitive and stored it in the class's member variables.
*   **After:** The entire implementation was overhauled.
    *   The constructor now contains nested loops to iterate through every `mesh` in the glTF file and every `primitive` within each mesh.
    *   Inside the loops, it parses all vertex attributes (positions, normals, UVs, etc.) and indices for that primitive.
    *   It assembles the data into `core::njVertex` objects.
    *   It creates a `core::njPrimitive` with the vertices, indices, and material index.
    *   It creates a `core::njMesh` with the collected primitives and the mesh name from the file.
    *   The final `core::njMesh` objects are stored in the `meshes_` member variable.
    *   The old getter methods were removed and replaced with the `get_meshes()` implementation.

## Architectural Discussion and Final Design

During implementation, a key architectural decision was discussed: the role of the `MeshBuilder` class.

### Path A: Self-Contained Loaders

The initial implementation path involved making each asset loader (starting with `GLTFAsset`) fully self-contained. The loader would be responsible for both parsing the file and constructing the final, engine-ready `njMesh` objects. This would have made the `MeshBuilder` class redundant, leading to its removal.

*   **Pros:** Simpler, more direct code path for each loader.
*   **Cons:** Logic for constructing `njMesh` objects would need to be duplicated in each new asset loader created in the future (e.g., an `OBJLoader`).

### Path B: Generic MeshBuilder (Chosen Path)

An alternative path was proposed: preserving the `MeshBuilder` as a generic, central utility for constructing meshes.

*   In this design, asset loaders (`GLTFAsset`) are only responsible for parsing their respective file formats and outputting structured, but raw, geometry data.
*   This raw data is then passed to the `MeshBuilder`, which is solely responsible for converting it into a final `njMesh` object.
*   **Pros:** More flexible and extensible. Provides a single, consistent point for mesh construction, making it easier to add support for new file formats without duplicating code.
*   **Cons:** Adds a minor layer of abstraction between the loader and the final mesh.

### Decision

Path B was chosen to favor long-term architectural flexibility. The implementation was therefore changed to follow this path:

1.  `GLTFAsset` was refactored to parse `.glb` files and output a `std::vector<RawMeshData>`.
2.  `MeshBuilder` was refactored into a stateless utility with a static `build` method to convert `RawMeshData` into an `njMesh`.
3.  `loader.cpp` was updated to orchestrate this new flow.
4.  The `MeshBuilder`'s unit tests were updated to reflect its new, stateless design.

## Architectural Discussion and Final Design

During implementation, a key architectural decision was discussed: the role of the `MeshBuilder` class.

### Path A: Self-Contained Loaders

The initial implementation path involved making each asset loader (starting with `GLTFAsset`) fully self-contained. The loader would be responsible for both parsing the file and constructing the final, engine-ready `njMesh` objects. This would have made the `MeshBuilder` class redundant, leading to its removal.

*   **Pros:** Simpler, more direct code path for each loader.
*   **Cons:** Logic for constructing `njMesh` objects would need to be duplicated in each new asset loader created in the future (e.g., an `OBJLoader`).

### Path B: Generic MeshBuilder (Chosen Path)

An alternative path was proposed: preserving the `MeshBuilder` as a generic, central utility for constructing meshes.

*   In this design, asset loaders (`GLTFAsset`) are only responsible for parsing their respective file formats and outputting structured, but raw, geometry data.
*   This raw data is then passed to the `MeshBuilder`, which is solely responsible for converting it into a final `njMesh` object.
*   **Pros:** More flexible and extensible. Provides a single, consistent point for mesh construction, making it easier to add support for new file formats without duplicating code.
*   **Cons:** Adds a minor layer of abstraction between the loader and the final mesh.

### Decision

Path B was chosen to favor long-term architectural flexibility. The implementation was therefore changed to follow this path:

1.  `GLTFAsset` was refactored to parse `.glb` files and output a `std::vector<RawMeshData>`.
2.  `MeshBuilder` was refactored into a stateless utility with a static `build` method to convert `RawMeshData` into an `njMesh`.
3.  `loader.cpp` was updated to orchestrate this new flow.
4.  The `MeshBuilder`'s unit tests were updated to reflect its new, stateless design.
