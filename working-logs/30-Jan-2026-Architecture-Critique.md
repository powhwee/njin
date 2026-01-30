# Architecture Critique & Refactoring Roadmap

## Executive Summary

The `njin` repository currently exists in a "Pragmatic Transition" state. While the Core Engine (Renderer, ECS, Physics) has matured into a robust, high-performance Vulkan implementation, the Application Layer (`main.cpp`) has regressed from its original disparate architectural vision into a manual, somewhat fragile configuration script.

The recent v0.5 "Namespacing" update successfully solved critical **Data Integrity** issues (preventing asset collisions), but it highlighted the need to improve **Developer Experience** by decoupling the application logic from internal asset details.

---

## 1. The Original Intent: The "Lost" Scene Graph

Analysis of the repository's earliest history (Commit `959bd0f`) reveals a clear, data-driven vision that has not yet been fully realized.

*   **Evidence**: The presence of `njSceneReader`, `njScene`, and `schema/scene.schema.json`.
*   **The Vision**:
    *   A completely data-driven composition model.
    *   Scene files (`.json`) defining the hierarchy: *"Attach mesh 'rocket' to node 'player' at (0,0,0)"*.
    *   **Decoupling**: The C++ engine was designed to load a scene file without knowing the internal structure of the 3D assets. It was never intended for `main.cpp` to hardcode mesh names like `Object_0`.

**Verdict**: The original architecture aimed for a robust, industry-standard **Scene Graph** model where content (Assets) and logic (Code) were strictly separated.

## 2. The Current Reality: Logic as Configuration

As development focused on bringing up the complex Vulkan renderer, the high-level abstraction layer was bypassed. `main.cpp` has effectively become a procedural configuration file.

### Critical Issues

#### A. The "String Contract" Fragility
The system relies on a fragile implicit contract distributed across three layers:
1.  **Manifest (`main.meshes`)**: Defines Alias (`"rocket"`) → File (`"toy_rocket.glb"`).
2.  **Loader Logic**: Internally concatenates Alias + GLTF Node Name → Registry Key (`"rocket-Object_0"`).
3.  **Application Logic (`main.cpp`)**: Hardcodes the expectation of that key (`.mesh = "rocket-Object_0"`).

**Risk**: This leaks implementation details (the internal node names of a 3D model) into the source code. If an artist renames a mesh in Blender from `Object_0` to `Fuselage`, the C++ code effectively breaks (or renders nothing) without a compile-time error.

#### B. Verbosity & Manual Management
`main.cpp` is tasked with low-level scene assembly that distracts from actual game logic:
*   **Boilerplate**: It takes ~50+ lines of code just to instantiate a camera, a static object, and a player.
*   **Manual Math**: Complex quaternion math and matrix composition (e.g., rotating the rocket part) are currently performed manually in the main loop. This logic belongs in a `TransformSystem` or `SceneGraphSystem`.

#### C. The Refactoring "Band-Aid" (v0.5)
The v0.5 update introduced **Namespacing** (prefixing assets with their alias).
*   **Success**: It completely solved the "Material Collision" bug where different GLTF files would overwrite each other's textures.
*   **Trade-off**: It solidified the coupling. `main.cpp` must now explicitly know the namespacing rule to request assets.

---

## 3. Recommendations & Roadmap

To realign the codebase with its original robust vision, I recommend a phased refactoring approach.

### Phase 1: The "Composite Pattern" (Low Effort, High Value)
**Goal**: Remove internal mesh names (e.g., `Object_0`) from `main.cpp`.

*   **Action**: Create a helper function `spawn_model_from_alias(engine, "rocket", position)`.
*   **Logic**: Use the existing `mesh_registry.get_all_mesh_names("rocket")` function to find *all* meshes associated with that alias and spawn entities for them automatically.
*   **Result**: `main.cpp` becomes agnostic to the GLTF structure.
    ```cpp
    // Before
    spawn_entity("rocket-Object_0", pos);
    
    // After
    spawn_model("rocket", pos); // Spawns correct mesh(es) automatically
    ```

### Phase 2: Revive the Scene Loader (Medium Effort)
**Goal**: Remove manual entity setup code.

*   **Action**: Re-implement a simplified version of the original `njSceneReader`.
*   **Implementation**: Create a `main.scene` file:
    ```json
    {
      "entities": [
        { "alias": "rocket", "pos": [5, 0, 0], "rot": [0, -90, 0] },
        { "alias": "player", "pos": [0, 1, 0] }
      ]
    }
    ```
*   **Result**: `main.cpp` reduces to initializes systems and calling `LoadScene("main.scene")`.

### Phase 3: Push Constant & Shader Safety (High Effort)
**Goal**: Eliminate "Invisible" bugs caused by data misalignment.

*   **Observation**: The `engine_patterns.md` notes that Push Constants are unchecked void pointers.
*   **Action**: Implement a "Shared Struct" header that is valid C++ *and* valid GLSL (via macros), ensuring that `struct PushConsts` in C++ always matches `layout(push_constant)` in shaders byte-for-byte.

## Conclusion

The engine has strong "bones." The rendering backend is explicit, performant, and safe. The "rot" is primarily in the interface between the Engine and the Game Logic. By moving back towards the original **Data-Driven** intent, we can make the engine significantly easier to use and less brittle to asset changes.
