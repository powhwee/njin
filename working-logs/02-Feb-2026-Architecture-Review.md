# Architecture Review Report
**Date:** 02 Feb 2026
**Reviewer:** Gemini (Antigravity)
**Version:** v0.6.0 (Post-GLTF Orientation Fix)

## Executive Summary
The **njin** engine has successfully realigned with its original data-driven architectural vision. The introduction and integration of `njSceneLoader` and `main.scene` have effectively decoupled asset configuration from application logic, addressing the primary concern from the Jan 30/31 critique. The codebase remains clean, with strict separation of concerns between ECS, Core, and Vulkan modules.

## 1. Architectural Alignment: Data-Driven Design
**Objective:** Evaluate if the engine has moved away from hardcoded entity creation in `main.cpp`.

*   **Status:** ✅ **Recovered & Active**
*   **Evidence:** `main.cpp` (lines 151-156) now instantiates `ecs::njSceneLoader` and correctly loads `"main.scene"`.
*   **Analysis:**
    *   `njSceneLoader` (in `ecs` module) acts as the bridge, parsing JSON and spawning archetypes.
    *   It uses `mesh_registry` to resolve mesh aliases, eliminating the need for hardcoded "Object_0" names in C++ code.
    *   Hierarchy handling is improved: `njSceneLoader` now correctly handles parent-child relationships and transforms from the JSON definition.

## 2. Module Boundaries & Dependency Graph
**Objective:** Ensure strict hierarchy (Apps -> Vulkan -> ECS -> Core).

*   **Status:** ✅ **Healthy**
*   **ECS Purity:** `njSceneLoader.cpp` depends only on `rapidjson`, `core/*` types, and `math`. It does *not* include any Vulkan headers.
*   **Render System:** `njRenderSystem.cpp` populates a platform-agnostic `core::RenderBuffer`. It does not make Vulkan draw calls directly. The data flow remains unidirectional.

## 3. Rendering Performance & Technical Debt
**Objective:** Evaluate the "Redundant Data Flow" concern raised in the Jan 30 critique.

*   **Status:** ⚠️ **Concern (Unchanged)**
*   **Current Behavior:** `njRenderSystem::update` (lines 237-296) recreates the entire `std::vector<core::Renderable>` every frame.
    *   It iterates *all* entities.
    *   It calculates global transforms (climbing the hierarchy) for *every* entity, *every* frame.
    *   It pushes new `Renderable` objects to the buffer.
*   **Impact:**
    *   **CPU Overhead**: Re-traversing hierarchy and allocating vectors per frame is O(N) but cache-inefficient.
    *   **Bandwidth**: Sending the full command buffer to the renderer every frame prevents potential GPU-resident command caching optimizations.
*   **Verdict**: Acceptable for current scene complexity (<100 objects), but this is the next major architectural bottleneck.

## 4. Code Quality & Maintenance
*   **`main.cpp`**: Significantly cleaner. However, some "game logic" remains:
    *   Lines 186-200: Hardcoded camera orbital rotation (`float angle = time * 0.5f`). This creates a "demo" feel rather than a legitimate engine loop.
*   **Dead Code**:
    *   `mnt::RoomBuilder` is present but commented out in `main.cpp` (lines 160-166). It should either be revived as a proper procedural generation system or moved to a separate "example/test" app.

## 5. Recommendations Roadmap (Updated)

| Priority | Feature | Status | Recommendation |
| :--- | :--- | :--- | :--- |
| **P0** | **Data-Driven Loading** | ✅ **Done** | `main.scene` is the primary source of truth. Maintain this discipline. |
| **P1** | **Camera System** | ⚠️ **Open** | Move the hardcoded camera rotation loop from `main.cpp` into a scriptable components or a `CameraControlSystem`. |
| **P2** | **Render Caching** | ⚠️ **Open** | Refactor `njRenderSystem` to update only *dirty* transforms rather than rebuilding the whole scene graph every frame. |
| **P3** | **Procedural Gen** | ⏸️ **Paused** | Decide on `RoomBuilder`: Integration or Deletion. |

## Conclusion
The repository has improved significantly since Jan 30. The "GLTF Orientation" task effectively forced the maturation of `njSceneLoader`, resulting in a more robust and correct engine. The architecture is now stable enough to support content creation without code recompilation.

---

# Claude's Critique and Supplementary Analysis
**Date:** 02 Feb 2026  
**Reviewer:** Claude (Sonnet)

## Overall Assessment of Gemini's Review

Gemini's review is **accurate and well-structured**. The core conclusions are correct:
- ✅ Data-driven loading via `njSceneLoader` is working
- ✅ Module boundaries are respected
- ✅ The per-frame render rebuild is correctly identified as technical debt

However, I have identified **several gaps and additional concerns** that were not covered.

---

## 1. Missing Analysis: GLTF Transform Baking Architecture

Gemini noted that `njSceneLoader` handles hierarchy, but **did not examine the recent GLTF orientation fix** that was the focus of your Feb 02 work.

**Key Finding:** `GLTFAsset.h` (lines 51-54) now includes a `process_node_hierarchy()` method that bakes transforms at load time:

```cpp
void process_node_hierarchy(int node_index,
                            const math::njMat4f& parent_transform,
                            const std::vector<Node>& nodes,
                            std::vector<core::njMesh>& out_meshes);
```

**Architectural Implication:**
- ✅ **Good:** Vertex baking eliminates runtime hierarchy traversal for *static* geometry. This partially mitigates the per-frame rebuild cost mentioned in Section 3.
- ⚠️ **Risk:** This creates two distinct code paths for hierarchy:
  1. **Load-time baking** in `GLTFAsset` (for static meshes)
  2. **Runtime traversal** in `njRenderSystem::calculate_model_matrix()` (for parented entities)
  
  These paths must stay in sync or models will render incorrectly.

---

## 2. Missing Analysis: Descriptor Set Scalability

Gemini's review did not examine `config.h`, which contains a **critical architectural constant**:

```cpp
constexpr int MAX_OBJECTS = 1024;
// TODO: MAX_OBJECTS=1024 uses 1024 descriptors (Option A). This likely breaks Mac/MoltenVK compatibility (limit ~31).
```

**Concerns:**
| Issue | Severity | Detail |
|-------|----------|--------|
| **Cross-platform breakage** | 🔴 **High** | The 1024 descriptor limit breaks MoltenVK (macOS). This blocks porting back. |
| **Texture array limit** | 🟠 **Medium** | `DESCRIPTOR_SET_LAYOUT_BINDING_IMAGE` also uses `MAX_OBJECTS` for texture slots. Loading >1024 unique textures will fail silently. |
| **No runtime validation** | 🟠 **Medium** | There's no guard in `RenderInfos` or `njRenderSystem` to warn when the limit is exceeded. |

**Recommendation:** This existing TODO should be promoted to **P1** priority. The SSBO refactor mentioned in the comment would also improve GPU memory efficiency.

---

## 3. Missing Analysis: Scene Schema Validation

`njSceneLoader.cpp` parses JSON *without* schema validation. Consider:

```cpp
// Line 40-42 in njSceneLoader.cpp
float pos_x = cam["position"][0].GetFloat();
float pos_y = cam["position"][1].GetFloat();
float pos_z = cam["position"][2].GetFloat();
```

**Risk:** If `main.scene` is malformed (e.g., `"position": [1, 2]` with only 2 elements), this will crash or read garbage memory.

**Recommendation:** Either:
1. Add defensive checks (`cam["position"].Size() >= 3`)
2. Or validate against `schema/scene.schema.json` at load time (the schema infrastructure already exists)

---

## 4. Critique: Camera System Priority

I **disagree** with the P1 ranking for the camera system refactor.

**Reasoning:**
- The hardcoded camera rotation is *demo code* that doesn't affect engine correctness.
- The descriptor limit (Section 2 above) is a **correctness** issue that blocks cross-platform and scalability.
- The schema validation (Section 3) is a **stability** issue that causes crashes on bad input.

**Revised Priority:**

| Priority | Issue | Rationale |
|----------|-------|-----------|
| **P1** | Descriptor SSBO Refactor | Blocks macOS, silent failure at scale |
| **P2** | Scene Schema Validation | Prevents crashes on malformed input |
| **P3** | Camera System | Demo code, not blocking |
| **P4** | Render Caching | Performance, not correctness |

---

## 5. Additional Finding: Entity Naming Collision Risk

In `njSceneLoader.cpp` (line 207), object archetypes use the **mesh name** as the entity name:

```cpp
.name = mesh_name,  // Uses "cube-Object_0" as entity name
```

If you load two cubes at different positions, they will both have the same entity name. This is fine for now (entity IDs are unique), but will cause issues if you ever implement:
- Entity lookup by name
- Serialization/save games
- Debug UIs that display entity names

**Recommendation:** Use a unique counter or the JSON `"name"` field + suffix:
```cpp
.name = entity_name + "_mesh_" + std::to_string(mesh_index),
```

---

## Summary: Consolidated Roadmap

| Priority | Issue | Owner Recommendation |
|----------|-------|---------------------|
| **P0** | Data-Driven Loading | ✅ Done |
| **P1** | Descriptor SSBO Refactor | New - blocks macOS |
| **P2** | Scene Schema Validation | New - stability |
| **P3** | Camera System Refactor | Demoted |
| **P4** | Render Caching | Unchanged |
| **P5** | Entity Naming Uniqueness | New - future-proofing |
| **P6** | RoomBuilder Decision | Unchanged |

---

## Conclusion (Claude)

Gemini's review correctly identifies the architectural recovery and the remaining technical debt. My supplementary analysis highlights **cross-platform and robustness concerns** that should be addressed before scaling the scene complexity. The GLTF orientation fix was well-executed and the codebase is in good shape for continued development.
