# njin Engine: Handover Document
**Date:** 31 January 2026  
**Prepared by:** Maintenance Developer  
**Purpose:** Summarize changes made during the Windows porting phase and provide recommendations for future development.

---

## Executive Summary

The njin engine was ported from macOS to Windows. During this process, several enhancements were made to the rendering pipeline and asset loading system. The core ECS architecture remains **unchanged** from your original design.

---

## What Was Changed

### 1. Asset Loading & Registry
| Change | Files Modified | Reason |
|--------|---------------|--------|
| Material namespacing | `njRenderSystem.cpp`, `njObjectArchetype.h/.cpp` | Prevent name collisions when loading multiple glTF files |
| `get_primary_mesh_name()` | `njRegistry.h/.tpp` | Avoid hardcoding internal glTF mesh names (e.g., `"Object_0"`) |
| Multi-mesh loading | `njRenderSystem.cpp` | Support glTF files with multiple meshes per model |
| Base color factor support | `njRenderSystem.cpp` | Read `base_color_factor` from materials for correct coloring |

### 2. Rendering Pipeline
| Change | Files Modified | Reason |
|--------|---------------|--------|
| Buffer limit increase | Vulkan config | Support more objects in scene |
| Push constant material colors | Shader/pipeline | Pass material color per-draw instead of per-object |

### 3. Platform Fixes
| Change | Reason |
|--------|--------|
| Removed `VK_KHR_portability_subset` | Mac-only extension, not needed on Windows |
| SDL3 initialization fixes | Different behavior on Windows |
| Exception handling in `main.cpp` | Better crash diagnostics |

---

## What Was NOT Changed

| Component | Status |
|-----------|--------|
| **ECS Framework** | Unchanged (`njEntityManager`, `njComponentMap`, archetypes) |
| **Data Structures** | Unchanged (`njMesh` → `njPrimitive` → `njVertex`) |
| **System Architecture** | Unchanged (`njRenderSystem`, `njPhysicsSystem`, `njSceneGraphSystem`) |
| **Physics/BVH** | Unchanged |

Your original design diagram (`njObject` → `njMesh` → `njPrimitive` → `njVertex`) is preserved exactly in the data layer. The ECS implementation you built is intact.

---

## Current State of the Codebase

### Working
- ✅ glTF model loading with multi-mesh support
- ✅ Material colors and textures rendering correctly
- ✅ Player archetype with physics
- ✅ Camera orbiting and projection

### Dead Code (Needs Decision)
| Code | Location | Issue |
|------|----------|-------|
| `RoomBuilder` | `njin/mnt/src/RoomBuilder.cpp` | Not called from `main.cpp`. Uses hardcoded mesh name `"cube"` which won't resolve. |
| `njScene`/`njSceneReader` | Only in `dist/test/` | Source files missing or never migrated. Test artifacts remain. |

---

## Recommendations

The following roadmap aligns with your original data-driven vision:

| Priority | Task | Description |
|----------|------|-------------|
| **P1** | Fix or remove `RoomBuilder` | **DONE**. Updated to configurable mesh/texture params. Verification passed. |
| **P2** | Create `main.scene` config | **DONE**. Replaced hardcoded entity creation in `main.cpp` with JSON configuration. |
| **P3** | Implement `njSceneLoader` | **DONE**. Implemented in `ecs` module to resolve circular dependency. Spawns entities from `main.scene`. |
| **P4** | Clean up test artifacts | **DONE**. Replaced legacy `njScene` artifacts with new unit tests (`njSceneLoader_test.cpp`). |

---

## Git History Summary (ECS Changes)

```
03c5139 feat: add multi-mesh loading, base_color_factor support
6f6dbcb (v0.5) fix: Material registry namespacing
f6c77d1 fix: Resolve rendering bugs and unresponsive application
e289294 (0.1) fix(ecs): Handle SDL_QUIT event for graceful shutdown
8efcc8a feat: implement billboard rendering
f5ad918 feat: add orthographic projection option for camera
b36fec7 refactor: change the way pipelines are assembled
959bd0f init (ECS fully formed from start)
```

---

## Files to Review

For a quick re-familiarization, review these key files:
- `njin/main.cpp` – Current application entry point
- `njin/ecs/src/njRenderSystem.cpp` – Rendering logic (most changes here)
- `njin/core/include/core/njRegistry.h` – New helper methods added
- `working-logs/` – Detailed session logs from debugging and porting

---

## Contact

If you have questions about specific changes, the `working-logs/` directory contains detailed debugging sessions with rationale for each fix.
