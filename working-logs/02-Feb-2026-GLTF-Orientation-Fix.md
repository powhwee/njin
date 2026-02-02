# GLTF Orientation Fix - Feb 2, 2026

## Problem
Stitch character model was loading with incorrect orientation (lying sideways instead of standing upright). This occurred even though the rendering pipeline was otherwise functional.

## Root Cause Analysis

### Why Previous Fixes Didn't Apply
The engine had prior row-major/column-major discussions around the **rendering pipeline** (MVP matrix construction, shader uniforms). Those were already correct. This issue was in **new code** introduced to parse GLTF node hierarchy transforms.

### The Actual Bug
The original `GLTFAsset.cpp` ignored the GLTF `"nodes"` array entirely. It loaded meshes directly from the `"meshes"` array without applying any node transforms.

GLTF files exported from Blender typically have a root node with a rotation that converts Z-up (Blender) to Y-up (engine standard). By ignoring nodes, the loader skipped this critical transform.

### Gemini's Initial Fix Attempt
Gemini added node parsing but made a critical error in matrix parsing:

```cpp
// WRONG - reads column-major GLTF data into row-vector constructor
math::njVec4f r0 { matrix[0], matrix[1], matrix[2], matrix[3] };   // This is Column 0, not Row 0!
math::njVec4f r1 { matrix[4], matrix[5], matrix[6], matrix[7] };
// ...
node.transform = math::njMat4f(r0, r1, r2, r3);
```

GLTF stores matrices in **column-major** order (indices 0-3 = column 0), but `njMat4` constructor expects **row vectors**. This required transposing during parsing.

## The Fix

### 1. Correct Matrix Parsing (Column→Row Transpose)
```cpp
// CORRECT - transpose during read
math::njVec4f r0 { matrix[0], matrix[4], matrix[8],  matrix[12] }; // Row 0 from Columns
math::njVec4f r1 { matrix[1], matrix[5], matrix[9],  matrix[13] };
math::njVec4f r2 { matrix[2], matrix[6], matrix[10], matrix[14] };
math::njVec4f r3 { matrix[3], matrix[7], matrix[11], matrix[15] };
node.transform = math::njMat4f(r0, r1, r2, r3);
```

### 2. Transform Baking
The loader now:
1. Parses all nodes from GLTF JSON
2. Recursively traverses hierarchy from scene root nodes
3. Computes global transform (parent × local) at each node
4. Bakes the transform into mesh vertices: position, normal, tangent

### 3. Unique Mesh Naming for Instancing
When a mesh is referenced by multiple nodes (common in rigged characters), each baked instance needs a unique name to avoid registry collisions:
```cpp
std::string processed_name = raw_mesh.name + "_" + node.name; // e.g., "Body_Armature"
```

### 4. Additional Compilation Fixes
- Added missing `#include "math/njMat4.h"` to `GLTFAsset.h`
- Fixed `v.normal = new_norm.normalize()` → `v.normal = math::normalize(new_norm)` (free function, not method)

### 5. Scene Cleanup
Removed manual workaround from `main.scene`:
```diff
- "rotation_x_degrees": -90.0,
```

## Why This Is Different From Previous Matrix Issues

| Context | Matrix Flow | Status |
|---------|-------------|--------|
| Rendering (MVP) | `njMat4` → push constant → shader | Already correct |
| GLTF Node Transform Parsing | GLTF JSON → `njMat4` | **Fixed today** |

The rendering matrices were always internally consistent. This bug was specifically in the **ingestion path** from external GLTF data.

## Mesh Naming Layers (Clarification)

| Layer | Purpose | Example |
|-------|---------|---------|
| `main.meshes` | Maps entity alias → asset alias | `"player": "stitch"` |
| `njSceneLoader` | Uses `mesh_alias` from entity config | `"mesh_alias": "player"` |
| `GLTFAsset` baking | Internal registry key | `"Body_Armature"` (unique per node) |

The new `MeshName_NodeName` pattern is purely internal and doesn't affect scene configuration.

## Files Changed
- `njin/util/src/GLTFAsset.cpp` - Complete rewrite of mesh loading with hierarchy support
- `njin/util/include/util/GLTFAsset.h` - Added `Node` struct, `njMat4.h` include
- `main.scene` - Removed manual rotation hack

## Verification
- Build: ✅ Success
- Runtime: ✅ Application launches without crash
- Visual: ✅ Character renders upright (confirmed by user)
