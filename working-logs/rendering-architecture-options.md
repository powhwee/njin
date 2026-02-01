# Rendering Architecture: Option A vs Option B

## Context
We needed to support rendering >16 objects.
- **Problem:** `MAX_OBJECTS=16` hardcoded limit.
- **Current State (v0.6):** Increased limit to 1024 using **Option A**.

## Option A: Descriptor Array (Current Implementation)
**Mechanism:** `descriptor_count = 1024`
- Creates an array of 1024 buffer descriptors in the shader: `buffer Model { ... } models[1024];`
- Each object gets its own descriptor pointing to a slice of memory.

| Pros | Cons |
| :--- | :--- |
| **Simple to Implement:** Minimal code changes (just constants). | **Mac Incompatible:** Exceeds `maxPerStageDescriptorStorageBuffers` (often ~31 on MoltenVK/iOS). |
| **Works on Windows:** Modern desktop GPUs allow 1000+ descriptors easily. | **High Overhead:** Managing 1024 descriptors is less efficient than 1. |
| | **Hard Limit:** Capped at `descriptor_count`, not scalable to 100k objects without changing config. |

## Option B: Single SSBO (Recommended for Future)
**Mechanism:** `descriptor_count = 1`
- Creates a SINGLE large buffer containing an array: `buffer Model { mat4 models[]; } objectData;`
- Shader indexes into the array: `objectData.models[gl_InstanceIndex]`.

| Pros | Cons |
| :--- | :--- |
| **Mac Compatible:** Uses only **1 descriptor**, safe on all platforms (Metal, mobile). | **Refactor Required:** Need to rewrite `shader.vert` and buffer update logic. |
| **Scalable:** Supports limited only by VRAM (millions of objects). | **Cognitive Load:** Requires thinking about buffer packing/alignment (std430), but simpler shader code overall. |
| **Performance:** Faster binding (1 descriptor vs 1000). | |

## Recommendation
Transition to **Option B** before shipping on macOS.
