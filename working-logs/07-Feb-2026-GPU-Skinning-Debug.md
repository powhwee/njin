# GPU Skinning Implementation & Debug Session

**Date:** 07 February 2026 | **Base Version:** v0.6.7 | **Focus:** Animation system wiring, GPU skinning pipeline, crash debugging

---

## Session Overview

This session completed the GPU skinning implementation started in a prior session and debugged a runtime crash (`invalid unordered_map<K, T> key`) that prevented the application from rendering with the new animation/skinning code.

---

## 1. GPU Skinning Implementation (Prior Session)

The following changes were made across the codebase to enable GPU skinning:

### Core Layer (Data Structures)

| File | Change |
|:-----|:-------|
| `core/include/core/njAnimation.h` | **NEW** — `njKeyframe`, `njAnimationChannel`, `njAnimation` structs |
| `core/include/core/njSkeleton.h` | **NEW** — `njSkeletonNode`, `njSkeleton` structs with joint/IBM data |
| `core/include/core/njVertex.h` | Extended with `joints` (uvec4) and `weights` (vec4) fields |
| `core/include/core/Renderable.h` | `MeshData` extended with `joint_matrices` + `is_skinned` |

### ECS Layer (Systems & Components)

| File | Change |
|:-----|:-------|
| `ecs/include/ecs/Components.h` | Added `njNodeComponent`, `njSkeletonRefComponent`, `njAnimationComponent`, `njAnimationBindingsComponent` |
| `ecs/src/njAnimationSystem.cpp` | **NEW** — Advances animation time, interpolates keyframes, computes per-node pose matrices, calculates final joint matrices (`pose * IBM`) |
| `ecs/src/njAnimationInputSystem.cpp` | **NEW** — Maps SDL key presses to animation triggers (infrastructure for future multi-animation models) |
| `ecs/src/njRenderSystem.cpp` | Modified to read `njSkeletonRefComponent` → look up animated pose → attach `joint_matrices` to `MeshData` |
| `ecs/src/njSceneLoader.cpp` | Modified to parse skeleton/animation registries, create `njNodeComponent` hierarchy, spawn animated player entities |

### Vulkan Layer (Pipeline & Shaders)

| File | Change |
|:-----|:-------|
| `vulkan/include/vulkan/config.h` | Added `DESCRIPTOR_SET_LAYOUT_BINDING_JOINTS` (binding 2, storage buffer, 128 entries), extended push constants to 32 bytes (`joint_offset` + `joint_count`), vertex format to 80 bytes |
| `vulkan/src/RenderInfos.cpp` | Collects joint matrices from skinned `MeshData`, uploads to "joints" SSBO binding, tracks per-mesh `joint_offset`/`joint_count` |
| `vulkan/src/SubpassModule.cpp` | Extended push constants struct to include `joint_offset`/`joint_count` |
| `vulkan/include/vulkan/RenderInfos.h` | `MeshRenderInfo` extended with `joint_offset`/`joint_count` |
| `shader/shader.vert` | Added `joints`/`weights` inputs, `joint_matrices` SSBO (binding 2), skinning logic: blends 4 joint transforms weighted by vertex weights |

### Asset Loading

| File | Change |
|:-----|:-------|
| `util/src/GLTFAsset.cpp` | Extended to parse skin data (joints, inverse bind matrices), animation channels/keyframes |
| `util/src/Accessor.cpp` | Extended to read `VEC4` unsigned byte/short data for joint indices |
| `core/src/loader.cpp` | Extended `load_meshes` to accept skeleton/animation registries |

---

## 2. Crash Debugging — `invalid unordered_map<K, T> key`

### Symptoms

Application crashed on Frame 1 with:
```
Exception caught in main: invalid unordered_map<K, T> key
```

Vulkan validation layers unloaded, indicating the crash happened during rendering.

### Initial Hypothesis (Wrong)

The `descriptor_count` for the joints SSBO was originally set to `MAX_JOINTS * MAX_OBJECTS = 128 * 1024 = 131072`. This was suspected to exceed GPU descriptor limits.

**Fix attempted:** Reduced `descriptor_count` to `MAX_JOINTS = 128` in both `config.h` and `shader.vert`.

**Result:** Crash persisted. The descriptor count reduction was correct but wasn't the cause.

### Debugging — Adding Debug Prints

Added `std::cerr` debug prints before every map-accessed operation in `RenderInfos::write_data()`:

```cpp
std::cerr << "[DEBUG] write model matrices (" << model_matrices.size() << ")" << std::endl;
// ... write model ...
std::cerr << "[DEBUG] write view_projection" << std::endl;
// ... write vp ...
std::cerr << "[DEBUG] load main_draw vertices" << std::endl;
// ... load main_draw ...
std::cerr << "[DEBUG] load iso_draw vertices" << std::endl;
// ... load iso_draw ... ← CRASH HERE
std::cerr << "[DEBUG] write_data complete" << std::endl;  // ← NEVER REACHED
```

### Output

```
[DEBUG] write model matrices (1)
[DEBUG] write view_projection
[DEBUG] load main_draw vertices
[DEBUG] load main_draw indices
[DEBUG] load iso_draw vertices       ← Last print before crash
Unloading layer library ...
Exception caught in main: invalid unordered_map<K, T> key
```

### Root Cause Found

The crash was in `vertex_buffers.load_into_buffer("iso_draw", iso_vertices)` — **not** in the joints code at all!

**Git diff revealed:** The original committed code (v0.6.7) had the `iso_draw` buffer upload **intentionally removed** with a comment:
```cpp
// Note: iso_draw vertex buffer removed - using only 3D rendering
```

And `main.cpp` only registered `VERTEX_BUFFER_INFO_MAIN_DRAW` — no `iso_draw` vertex buffer existed in the resource map.

During the animation implementation, when "restoring" isometric rendering code that was accidentally overwritten, the `iso_draw` upload line was incorrectly re-added. This caused `VertexBuffers::load_into_buffer("iso_draw", ...)` to fail because the key `"iso_draw"` didn't exist in the vertex buffers map.

### Fix

1. **Removed** the `load_into_buffer("iso_draw", ...)` call
2. **Restored** the original comment: `// Note: iso_draw vertex buffer removed - using only 3D rendering`
3. **Reverted** `main.cpp` to only register `VERTEX_BUFFER_INFO_MAIN_DRAW`

### Key Insight: Isometric Camera vs. Billboard Pipeline

Two different features share the "iso" naming but are independent:

| Feature | How It Works | Status at v0.6.7 |
|:--------|:-------------|:-----------------|
| **Isometric camera** (P/O/I keys) | Orthographic projection on regular `main_draw` pipeline | ✅ Working |
| **Billboard pipeline** (`iso_draw`) | Separate rendering pipeline with own vertex format, shaders, vertex buffer | ❌ Disabled (infrastructure in `config.h`, upload removed) |

The isometric camera was working because it uses the regular 3D rendering pipeline with an orthographic matrix — it has nothing to do with the `iso_draw` billboard pipeline.

---

## 3. Animation Auto-Play Investigation

### Issue
Animation started automatically on launch instead of waiting for key press.

### Cause
`njSceneLoader.cpp` line 247: `anim_comp.playing = true;`

### Analysis
The `njAnimationInputSystem` infrastructure exists to map key presses to animations via `njAnimationBindingsComponent`, but:
- The scene loader **never creates** an `njAnimationBindingsComponent` for the player entity
- The player model only has **one animation** (`"Animation"`)
- Without bindings, setting `playing = false` would mean the animation never plays

### Decision
Kept `playing = true` (auto-play) as the practical choice for now. The key binding infrastructure is ready for when models with multiple animations are available.

---

## 4. Descriptor Architecture Conformance

Verified that the joints SSBO follows the **Option A (descriptor array)** pattern, consistent with the existing model matrices:

| Binding | Descriptor Type | Count | Pattern |
|:--------|:----------------|:-----:|:-------:|
| 0 — Model matrices | `VK_DESCRIPTOR_TYPE_STORAGE_BUFFER` | 1024 | Option A |
| 1 — View/Projection | `VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER` | 1 | Single |
| 2 — Joint matrices | `VK_DESCRIPTOR_TYPE_STORAGE_BUFFER` | 128 | Option A |

The original author's intent (Option A descriptor array) was respected. No single-SSBO (Option B) pattern was forced.

---

## 5. Files Modified This Session

| File | Lines Changed | Purpose |
|:-----|:-------------|:--------|
| `vulkan/include/vulkan/config.h:76` | 1 | Reduced joint descriptor count from 131072 to 128 |
| `shader/shader.vert:37` | 1 | Updated joint array size from 131072 to 128 |
| `vulkan/src/RenderInfos.cpp:335-345` | ~10 | Removed incorrect iso_draw upload, cleaned debug prints |
| `ecs/src/njSceneLoader.cpp:247` | 1 | Kept `playing = true` with updated comment |

---

## 6. Lessons Learned

1. **Always check git diff before "restoring" code** — The iso_draw upload was intentionally removed in the committed code, but was incorrectly re-added during the session
2. **Debug prints are essential** — The crash location was completely different from what was suspected. Targeted `std::cerr` prints immediately revealed the real failure point
3. **Name ambiguity causes confusion** — "Isometric" refers to both the camera mode AND the billboard rendering pipeline. These are independent features
4. **Descriptor count matters** — 131072 descriptors would have been problematic anyway, even if it wasn't the immediate crash cause

---

## 7. Current State

- ✅ Application runs without crash
- ✅ Animation plays automatically (single animation)
- ✅ GPU skinning pipeline fully wired (SSBO, push constants, shader)
- ✅ Architecture review completed — all new code respects layer boundaries
- ⚠️ Visual verification of skinning deformation still needed
- ⚠️ Debug logging (`std::cout` prints) should be cleaned up before release
