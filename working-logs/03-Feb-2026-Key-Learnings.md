# Key Learnings & Bug Fixes

**Date:** 03 February 2026

This document captures hard-won debugging knowledge. Each bug consumed significant time; understanding them prevents regression.

## Table of Contents

- [1. The Invisible Cube Saga (~6 hours)](#1-the-invisible-cube-saga-v051)
- [2. The "Orientation Saga" (~4 hours)](#2-the-orientation-saga-v061)
- [3. The "Washed Out" Rendering (~3 hours)](#3-the-washed-out-rendering-v062)
- [4. The "Tiny Dog" Scale Bug (~2 hours)](#4-the-tiny-dog-scale-bug-v063)
- [5. The 16-Object Limit (~1 hour)](#5-the-16-object-limit-v06)
- [6. Environment Setup (Recurring)](#6-environment-setup-recurring)
- [7. Matrix Column/Row Confusion (Recurring)](#7-matrix-columnrow-confusion-v061)
- [8. Workflow Learnings](#8-workflow-learnings)
- [9. The "Ghost Key" iso_draw Crash](#9-the-ghost-key-iso_draw-crash-v068)
- [10. Descriptor Count Sizing](#10-descriptor-count-sizing-v068)
- [11. Isometric vs Billboard Naming Confusion](#11-isometric-vs-billboard-naming-confusion-v068)
- [12. Animation System Bootstrap](#12-animation-system-bootstrap-v068)

---

## 1. The Invisible Cube Saga (v0.5.1)
*Time sink: ~6 hours (multiple cascading bugs)*

This was the most complex debugging session, revealing **6 independent bugs**:

| Bug | Symptom | Root Cause | Fix |
|:----|:--------|:-----------|:----|
| 1. Distorted visuals | Shrinking/expanding | `atan` instead of `tan` in projection | Fix formula |
| 2. Crash on player disable | App wouldn't quit | `njInputSystem` only checked events if input component existed | Always poll SDL_QUIT |
| 3. Index buffer crash | `out_of_range` on iso_draw | Assumed all subpasses have index buffers | Make index buffer optional in config |
| 4. Invisible geometry | Nothing rendered | `Accessor` ignored `byte_stride`, read garbage from interleaved buffers | Respect stride in accessor |
| 5. Compile error | `const` member assignment | Assigning to const in constructor body | Use lambda in initializer list |
| 6. Validation error | `VK_NULL_HANDLE` for index buffer | `Renderer` didn't pass buffer to `BindSet` | Fix wiring in constructor |

**Key Learning:** Multiple bugs can mask each other. Fix one → new symptom appears. Be systematic.

---

## 2. The "Orientation Saga" (v0.6.1)
*Time sink: ~4 hours across multiple sessions*

| | Details |
|:--|:--------|
| **Symptom** | Models (Stitch) loaded lying on their side |
| **Red Herrings** | Manual rotation hacks, shader matrix tweaks |
| **Root Cause** | GLTF loader ignored `nodes[]` array—only read raw `meshes[]`. Blender exports include a root node rotation (Z-up → Y-up) that was being skipped. |
| **Fix** | Parse full node hierarchy, compute global transforms, **bake into vertices** at load time. |
| **Files** | `GLTFAsset.cpp` (complete rewrite of mesh loading) |

**Key Learning:** GLTF `meshes` are in local space. `nodes` define the world-space placement. Always traverse the node tree.

---

## 3. The "Washed Out" Rendering (v0.6.2)
*Time sink: ~3 hours*

| | Details |
|:--|:--------|
| **Symptom** | Models appeared pale/desaturated |
| **Misdiagnosis** | "Aggressive tone mapping" (partially correct) |
| **Root Causes** | **1)** Double gamma: sRGB swapchain + manual `pow(x,1/2.2)` in shader. **2)** Rim lighting adds white, desaturating colors. |
| **Fix** | Remove manual gamma (sRGB swapchain handles it), remove rim lighting. |
| **Files** | `shader.frag` |

**Debug Process:**
1. Remove tone mapping → still wrong
2. Remove manual gamma → better, not fixed
3. Binary search lighting effects → **rim lighting** was culprit

**Key Learning:** `VK_FORMAT_*_SRGB` swapchain = no manual gamma in shader. Additive white effects (rim, bloom) desaturate.

---

## 4. The "Tiny Dog" Scale Bug (v0.6.3)
*Time sink: ~2 hours*

| | Details |
|:--|:--------|
| **Symptom** | `"scale": 35.0` in scene JSON had no effect |
| **Root Cause** | `njPhysicsSystem::calculate_new_transforms()` constructed a **fresh translation matrix** each frame, discarding the original scale/rotation. |
| **Fix** | Modify existing transform matrix, only update translation column `[x][3]`. |
| **Files** | `njPhysicsSystem.cpp` |

```cpp
// BEFORE (wrong): Created pure translation, lost scale
math::njMat4f new_transform{ Translation, { x, y, z } };

// AFTER (correct): Preserve scale/rotation, update translation only
math::njMat4f new_transform = global_transform;
new_transform[0][3] = x; new_transform[1][3] = y; new_transform[2][3] = z;
```

---

## 5. The 16-Object Limit (v0.6)
*Time sink: ~1 hour*

| | Details |
|:--|:--------|
| **Symptom** | Only first 16 entities rendered |
| **Root Cause** | `MAX_OBJECTS=16` hardcoded in `config.h`, `shader.vert`, `shader.frag` |
| **Fix** | Increased to 1024 |
| **Caveat** | Breaks macOS (MoltenVK ~31 descriptor limit). Future: SSBO refactor. |

---

## 6. Environment Setup (Recurring)
*Every session on macOS*

| | Details |
|:--|:--------|
| **Symptom** | "VK_KHR_surface not supported", "Validation layers not available" |
| **Root Cause** | Missing `VK_LAYER_PATH`, `VK_ICD_FILENAMES` environment variables |
| **Fix** | `source setup-env-personal.sh` before running |

**Key Learning:** IDE terminals don't inherit these. Always source the script.

---

## 7. Matrix Column/Row Confusion (v0.6.1)
*Recurring conceptual issue*

| | Details |
|:--|:--------|
| **Symptom** | Transforms applied incorrectly (wrong axis, orientation) |
| **Root Cause** | GLTF is column-major, njMat4 constructor takes row vectors |
| **Fix** | Transpose during parsing: `r0 = {m[0], m[4], m[8], m[12]}` |

This is now documented in architecture_review.md as institutional knowledge.

---

## 8. Workflow Learnings

| Learning | Details |
|:---------|:--------|
| **Diff-First** | Review diffs before applying—catches logic errors early |
| **Binary Search** | When N things could cause issue, remove systematically |
| **Validation Layers** | Always enable—catches null handles, format mismatches |
| **Data-Driven** | `main.scene` JSON >> hardcoded `main.cpp`—faster iteration |

---

## 9. The "Ghost Key" iso_draw Crash (v0.6.8)
*Time sink: ~1.5 hours*

| | Details |
|:--|:--------|
| **Symptom** | `invalid unordered_map<K, T> key` crash on Frame 1 |
| **Red Herrings** | Suspected joints SSBO descriptor count (131072 too large), suspected animation system issues |
| **Root Cause** | `vertex_buffers.load_into_buffer("iso_draw", ...)` called, but `"iso_draw"` key was never registered in `main.cpp`'s `RenderResourceInfos`. The upload line had been intentionally removed at v0.6.7 but was accidentally re-added during animation implementation. |
| **Fix** | Remove the `iso_draw` buffer upload line (it was already intentionally disabled). |
| **Files** | `RenderInfos.cpp`, `main.cpp` |

**Debug Process:**
1. Added `std::cerr` prints before every map-accessed call in `write_data()`
2. Output showed crash after `"load iso_draw vertices"` — immediately pinpointing the failing key lookup
3. `git show HEAD:njin/vulkan/src/RenderInfos.cpp` confirmed the upload was already removed at v0.6.7

**Key Learning:** When "restoring" code that was overwritten, always check `git show HEAD:<file>` to see what the committed state actually was. Code may have been intentionally removed.

---

## 10. Descriptor Count Sizing (v0.6.8)

| | Details |
|:--|:--------|
| **Issue** | Joints SSBO descriptor count was set to `MAX_JOINTS * MAX_OBJECTS = 128 * 1024 = 131072` |
| **Risk** | Descriptor pool exhaustion, exceeding GPU limits |
| **Fix** | Reduced to `MAX_JOINTS = 128` — the Option A descriptor array pattern only needs one array of N entries, not N × M |

**Key Learning:** In Option A (descriptor array), `descriptor_count` = array size, not array size × number of users. Model matrices use 1024 descriptors for 1024 objects; joints should use 128 descriptors for 128 joints, not 128 × 1024.

---

## 11. Isometric vs Billboard Naming Confusion (v0.6.8)

| | Details |
|:--|:--------|
| **Confusion** | "Isometric" camera and "iso_draw" billboard pipeline share the "iso" prefix but are completely independent features |
| **Isometric camera** | Works via orthographic projection matrix on the regular `main_draw` pipeline (P/O/I key toggles) |
| **Billboard pipeline** | Separate rendering pipeline (`iso_draw`) with own vertex format — infrastructure exists in `config.h` but buffer upload was disabled at v0.6.7 |

**Key Learning:** Name ambiguity between features causes debugging confusion. The isometric camera has nothing to do with the iso_draw billboard pipeline.

---

## 12. Animation System Bootstrap (v0.6.8)

| | Details |
|:--|:--------|
| **Issue** | Animation auto-plays on launch instead of waiting for key press |
| **Cause** | `njSceneLoader` sets `anim_comp.playing = true` but never creates `njAnimationBindingsComponent` (no key bindings) |
| **Decision** | Kept `playing = true` because the test model has only 1 animation and no key bindings are configured |

**Key Learning:** When building an input-driven system, verify the full chain: input system → bindings component → state component. If bindings are missing, the input system is dead code and the default state determines behavior.

