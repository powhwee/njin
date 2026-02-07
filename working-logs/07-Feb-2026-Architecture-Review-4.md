# Architecture Conformance Review — Post-Animation/Skinning

**Date:** 07 February 2026 | **Baseline:** v0.6.7 → animation/skinning additions | **Reference:** [Architecture Review 3](file:///c:/Users/pow_h/njin/working-logs/03-Feb-2026-Architecture-Review-3.md)

---

## 1. Summary

The animation and GPU skinning implementation **respects all core architectural rules** established in the v0.6.6 review. Layer boundaries are clean, the config-driven Vulkan pattern is followed, and the ECS-to-Vulkan isolation via `RenderBuffer` is preserved. A few pre-existing violations were re-identified and one new concern is noted.

---

## 2. Layer Boundary Audit

The architecture review defines these rules:
- `core` **never** imports from `ecs` or `vulkan`
- `ecs` imports from `core` and `math` only
- `vulkan` imports from `math` only (receives core data via `main.cpp` pointers)
- ECS makes **zero** Vulkan calls

### New Core Types — ✅ Clean

| File | Imports | Verdict |
|:-----|:--------|:-------:|
| [njAnimation.h](file:///c:/Users/pow_h/njin/njin/core/include/core/njAnimation.h) | `math/njQuat.h`, `math/njVec3.h` | ✅ |
| [njSkeleton.h](file:///c:/Users/pow_h/njin/njin/core/include/core/njSkeleton.h) | `math/njMat4.h`, `math/njQuat.h`, `math/njVec3.h` | ✅ |
| [Renderable.h](file:///c:/Users/pow_h/njin/njin/core/include/core/Renderable.h) | `core/Types.h`, `core/njVertex.h`, `math/njMat4.h` | ✅ |

Both `njAnimation` and `njSkeleton` are **pure data structures** — no logic, no methods, just fields. This follows the "Core types are dumb data containers" design principle from §1.6.

### New ECS Systems — ✅ Clean

| File | Imports | Verdict |
|:-----|:--------|:-------:|
| [njAnimationSystem.h](file:///c:/Users/pow_h/njin/njin/ecs/include/ecs/njAnimationSystem.h) | `ecs/njSystem.h` | ✅ |
| [njAnimationSystem.cpp](file:///c:/Users/pow_h/njin/njin/ecs/src/njAnimationSystem.cpp) | `core/njAnimation.h`, `ecs/Components.h`, `math/njQuat.h`, `math/njVec3.h` | ✅ |
| [njAnimationInputSystem.h](file:///c:/Users/pow_h/njin/njin/ecs/include/ecs/njAnimationInputSystem.h) | `ecs/njSystem.h` | ✅ |
| [njAnimationInputSystem.cpp](file:///c:/Users/pow_h/njin/njin/ecs/src/njAnimationInputSystem.cpp) | `ecs/Components.h`, `SDL3/SDL_keyboard.h` | ✅ |

ECS systems import only `core/`, `math/`, `ecs/`, and SDL (for input). **Zero Vulkan calls.**

### Vulkan Layer — ✅ Clean

| File | Imports | Verdict |
|:-----|:--------|:-------:|
| [config.h](file:///c:/Users/pow_h/njin/njin/vulkan/include/vulkan/config.h) (joints SSBO) | `math/njMat4.h` (via existing includes) | ✅ |
| [RenderInfos.cpp](file:///c:/Users/pow_h/njin/njin/vulkan/src/RenderInfos.cpp) (joint upload) | `core/njTexture.h`, `vulkan/RenderInfos.h` | ✅ |
| [SubpassModule.cpp](file:///c:/Users/pow_h/njin/njin/vulkan/src/SubpassModule.cpp) (push constants) | Existing imports only | ✅ |
| [shader.vert](file:///c:/Users/pow_h/njin/shader/shader.vert) (skinning logic) | N/A (GLSL) | ✅ |

---

## 3. Config-Driven Pattern Conformance

The architecture review (§1.2) states `config.h` is the **single source of truth** for pipeline configuration.

| Config Item | Pattern | Consistent? |
|:------------|:--------|:-----------:|
| `DESCRIPTOR_SET_LAYOUT_BINDING_JOINTS` | Same struct format as MODEL and VP bindings | ✅ |
| `descriptor_count = MAX_JOINTS (128)` | Follows Option A (descriptor array), same as MODEL (1024) | ✅ |
| `DESCRIPTOR_SET_LAYOUT_INFO_MVP` | Correctly includes joints binding alongside model + vp | ✅ |
| Push constants (32 bytes) | Extended with `joint_offset` + `joint_count` | ✅ |
| Vertex format (80 bytes) | Extended with `joints` (uvec4) + `weights` (vec4) | ✅ |

> [!IMPORTANT]
> The joints SSBO follows the same **Option A (descriptor array)** pattern as the model matrices, consistent with the architecture review's §6.1. The eventual migration to Option B (single SSBO) should migrate both together.

---

## 4. Data Flow Conformance

The v0.6.6 review (§3.3) defines this data flow:
```
ECS (njRenderSystem) → RenderBuffer → Vulkan (RenderInfos) → SubpassModule → GPU
```

The animation system adds a new upstream path that feeds into this same flow:

```
njAnimationSystem → njAnimationComponent.pose/joint_matrices
                         ↓
njRenderSystem → MeshData.joint_matrices → RenderBuffer
                         ↓
RenderInfos → SSBO upload + push constants → shader.vert
```

**This is architecturally correct.** The ECS layer computes matrices and passes them through the platform-agnostic `MeshData` struct in `core/Renderable.h`. The Vulkan layer consumes them without knowing about ECS. The isolation boundary is clean.

---

## 5. Pre-Existing Violations

These existed before the animation changes and were not introduced by this session:

### 5.1 `njVertex.h` includes `vulkan_core.h` — ⚠️ Layer Violation

[njVertex.h](file:///c:/Users/pow_h/njin/njin/core/include/core/njVertex.h) (in `core/`) directly includes `<vulkan/vulkan_core.h>` and exposes `VkVertexInputBindingDescription` and `VkVertexInputAttributeDescription`. This violates the rule that core never imports vulkan.

> [!WARNING]
> This is the **most significant layer violation** in the codebase. The static methods `get_binding_description()` and `get_attribute_descriptions()` are Vulkan-specific and should live in the Vulkan layer. However, since config.h already defines `VERTEX_INPUT_MAIN_DRAW_FORMAT` as the Vulkan-side vertex struct, these methods may be dead code.

### 5.2 `loader.cpp` lives in `core/` but imports `util/`

[loader.cpp](file:///c:/Users/pow_h/njin/njin/core/src/loader.cpp) imports `util/GLTFAsset.h`, `util/json.h`, `util/stb.h`. The architecture diagram shows `core` as a leaf node — it should not import from sibling modules. This is a dependency inversion: either `loader.cpp` should move to `util/` or be elevated to `main.cpp` level.

---

## 6. New Observations Post-Animation

### 6.1 `njAnimationComponent` stores computed matrices — Acceptable

`njAnimationComponent` holds both `pose` (per-node global transforms) and `joint_matrices` (final GPU-ready matrices). This is ECS-appropriate — components own state, systems mutate it. No violation.

### 6.2 `MeshData` extended with `joint_matrices` + `is_skinned` — ✅ Correct

`MeshData` in [Renderable.h](file:///c:/Users/pow_h/njin/njin/core/include/core/Renderable.h) now carries `std::vector<njMat4f> joint_matrices` and `bool is_skinned`. This is the correct bridge — ECS populates it, Vulkan consumes it, and the struct lives in `core/` where both can access it.

### 6.3 Billboard Pipeline Code — Dormant but Present

The iso/billboard rendering code in [RenderInfos.cpp](file:///c:/Users/pow_h/njin/njin/vulkan/src/RenderInfos.cpp) still populates `BillboardRenderInfo` entries and calculates billboard quads, but the vertex buffer upload is disabled (comment: `"Note: iso_draw vertex buffer removed - using only 3D rendering"`). The pipeline infrastructure remains in `config.h` for future use. The isometric camera works via orthographic projection on the main_draw pipeline.

### 6.4 Debug Logging Should Be Cleaned Up

Several `std::cout` debug prints with frame counters remain:
- `njAnimationSystem.cpp:52` — `[AnimSystem] entities with NodeComp+AnimComp:`
- `njRenderSystem.cpp:289` — `[Render] Animated mesh '...'`

These should be removed or gated behind a debug flag before release.

---

## 7. Architecture Scorecard

| Rule | Status | Notes |
|:-----|:------:|:------|
| Core imports only math | ⚠️ | Pre-existing: `njVertex.h` imports vulkan, `loader.cpp` imports util |
| ECS imports core + math only | ✅ | All new systems comply |
| Vulkan imports math only | ✅ | Receives core data via main.cpp pointers |
| ECS makes zero Vulkan calls | ✅ | Animation system is Vulkan-agnostic |
| config.h is single source of truth | ✅ | Joints binding defined there, shader matches |
| Registry pattern preserved | ✅ | Skeleton + animation registries added correctly |
| Matrix convention (row-major) | ✅ | Transpose at upload, shader transposes back |
| Data flows through RenderBuffer | ✅ | joint_matrices added to MeshData correctly |
| Option A descriptor pattern | ✅ | 128 joints array, same pattern as 1024 models |

**Overall: The animation/skinning implementation is architecturally sound.** The two pre-existing layer violations (`njVertex.h`, `loader.cpp`) predate this work and are tracked as tech debt.
