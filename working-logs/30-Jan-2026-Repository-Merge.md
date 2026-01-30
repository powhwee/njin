# Repository Merge: Windows Porting + Remote Features

**Date**: 30 January 2026  
**Topic**: Merging local Windows porting changes with remote `origin/main` features

---

## Summary

This session analyzed the architectural differences between the local working directory (Windows porting work) and the remote `origin/main` branch, then performed a careful merge to combine both sets of changes while maintaining cross-platform compatibility.

---

## Repository State Before Merge

### Local HEAD
- Commit: `d09f52f` (tag: v0.5.2)
- 1 commit behind `origin/main`
- 10 uncommitted files with Windows porting changes

### Remote HEAD  
- Commit: `03c5139` — "feat: add multi-mesh loading, base_color_factor support, and buffer limit increase"

---

## Key Differences Identified

### Remote Features (Missing Locally)
| Feature | Description |
|---------|-------------|
| Player Archetype | Active with physics, input, mesh lookup |
| Physics System | `njPhysicsSystem` added to engine |
| Dynamic Mesh Loading | `get_all_mesh_names()` for multi-part models |
| Base Color Factor | 24-byte push constants with RGBA material color |
| Buffer Limits | 500k vertices, 1.5M indices |
| New Model Assets | `porshe.glb`, `stitch_free.glb`, `fiat_punto.glb` |

### Local Windows Porting (Missing on Remote)
| Feature | Description |
|---------|-------------|
| SDL Main Handling | `SDL_MAIN_HANDLED`, `SDL_SetMainReady()` |
| SDL Init | `SDL_Init(SDL_INIT_VIDEO \| SDL_INIT_EVENTS)` with error check |
| Exception Handling | Try-catch wrapper around entire `main()` |
| Debug Logging | Verbose `std::cout` statements throughout |
| Camera Null Check | Defensive check for empty camera views |
| Window Resizable | `SDL_WINDOW_RESIZABLE` flag |

---

## GPU Compatibility Analysis

### The Problem
The Vulkan specification minimum for `maxPerStageDescriptorUniformBuffers` is **15**. NVIDIA GPUs (like GTX 1070 Ti) strictly enforce this limit, while MoltenVK on macOS may be more lenient.

> **Key Insight**: Remote's `view_projection.descriptor_count = 16` would fail on Windows NVIDIA drivers!

### Settings Decision

| Setting | Remote | Local | Chosen | Reason |
|---------|--------|-------|--------|--------|
| `view_projection` descriptor_count | 16 | **1** | **1** | Exceeds NVIDIA's 15 limit; only need 1 for camera |
| `image` descriptor_count | 64 | **16** | **16** | User confirmed Windows GPU hit limits at 64 |
| Push constant size | **24** | 8 | **24** | Enables base_color_factor; within 128-byte minimum |
| Max vertices | **500,000** | 100,000 | **500,000** | High-poly model support |
| Max indices | **1,500,000** | 300,000 | **1,500,000** | High-poly model support |

### Push Constant Sync Requirement

When using 24-byte push constants, these three files **MUST stay in sync**:

1. `config.h` — `PUSH_CONSTANT_RANGE_MAIN_DRAW_MODEL.size = 24`
2. `SubpassModule.cpp` — C++ struct with all 6 fields
3. `shader.frag` — GLSL struct with all 6 fields

```cpp
// C++ struct (SubpassModule.cpp)
struct {
    int model_index;      // 4 bytes
    int texture_index;    // 4 bytes
    float base_color_r;   // 4 bytes
    float base_color_g;   // 4 bytes
    float base_color_b;   // 4 bytes
    float base_color_a;   // 4 bytes
};                        // = 24 bytes total
```

```glsl
// GLSL struct (shader.frag)
layout (push_constant) uniform PushConstants {
    int model_index;
    int texture_index;
    float base_color_r;
    float base_color_g;
    float base_color_b;
    float base_color_a;
} pc;
```

---

## Merge Process

### Steps Executed
1. `git stash push -m "Windows porting changes"` — Saved local changes
2. `git pull origin main` — Pulled remote commit `03c5139`
3. `git stash pop` — Applied stashed changes
4. **Conflict Resolution** — `njin/main.cpp` had merge conflicts
5. **Manual Merge** — Combined both feature sets in `main.cpp`
6. **GPU Compat Fixes** — Reduced texture descriptor count to 16

### Merge Conflict in `main.cpp`

The conflict occurred because both branches modified the same sections:
- Remote: Added physics system, player archetype, dynamic mesh loop
- Local: Wrapped everything in try-catch, added debug logging, Windows SDL init

**Resolution**: Kept BOTH sets of changes by restructuring the file to include:
- Windows SDL init at the top (local)
- Try-catch wrapper (local)
- Debug logging (local)
- Physics system (remote)
- Dynamic mesh loading loop (remote)
- Player archetype with physics (remote)
- Camera null check (local)

---

## Files Changed After Merge

| File | Changes |
|------|---------|
| `njin/main.cpp` | Combined Windows init + remote features |
| `njin/vulkan/include/vulkan/config.h` | Texture descriptor_count → 16 |
| `shader/shader.frag` | Texture array → 16 |
| `njin/core/include/core/njTexture.h` | From stash |
| `njin/core/src/njTexture.cpp` | From stash |
| `njin/util/src/GLTFAsset.cpp` | From stash (`#undef GetObject` for Windows) |
| `njin/vulkan/include/vulkan/Window.h` | From stash |
| `njin/vulkan/src/LogicalDevice.cpp` | From stash |
| `njin/vulkan/src/Renderer.cpp` | From stash |

---

## Cross-Platform Considerations

### SDL Initialization
```cpp
// Windows-safe approach (works on both platforms)
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

SDL_SetMainReady();  // Required for Windows
if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
    return 1;
}
```

### MoltenVK Portability (Already Handled)
The codebase uses `#ifdef __APPLE__` guards for:
- `VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR`
- `VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME`
- `VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME`

### Windows Macro Collision
`GLTFAsset.cpp` includes:
```cpp
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#include <windows.h>
#ifdef GetObject
#undef GetObject
#endif
#endif
```
This prevents `windows.h`'s `GetObject` macro from colliding with RapidJSON's `GetObject()` method.

---

## Next Steps

1. **Build the project** — Verify merged code compiles on Windows
2. **Test rendering** — Confirm player archetype and physics work
3. **Commit** — `git commit -m "feat: merge Windows porting with remote features"`
4. **Push** — `git push origin main`

---

## Reference: Knowledge Items Used

- `vulkan_buffers.md` — GPU descriptor limits and buffer sizing
- `platform_differences.md` — Cross-platform SDL and Vulkan differences
- `rendering_debug.md` — Push constant alignment requirements
