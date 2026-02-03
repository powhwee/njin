# GLTF Enhancement Status

**Date:** 03 February 2026 | **Current Version:** v0.6.3

## 1. Feature Status Matrix

| # | Feature | Status | Version | Key Files |
|:-:|:--------|:------:|:-------:|:----------|
| 1 | Tangents | ✅ Done | v0.4 | `GLTFAsset.cpp` L416-426 |
| 2 | Component Types (u8/u16/u32) | ✅ Done | v0.4 | `Accessor.cpp` |
| 3 | Multi-Primitive/Mesh | ✅ Done | v0.5 | `GLTFAsset.cpp`, `enhancement-3.md` |
| 4 | Materials & Textures | ✅ Done | v0.5 | `process_materials()`, `njMaterial.h` |
| 5 | Node Hierarchy (Baked) | ✅ Done | v0.6.1 | `GLTFAsset.cpp` L475-648 |
| 6 | Cameras | ⚠️ Partial | — | Via `njSceneLoader`, not GLTF |
| 7 | Animations & Skinning | ❌ Pending | — | Requires runtime hierarchy |

## 2. The Baking Strategy

Rather than implementing a runtime scene graph, the engine **bakes** GLTF node transforms into vertex data at load time:

```
GLTF File → Parse Nodes → Compute GlobalTransform → Transform Vertices → Flat njMesh List
```

**How it works** (in `GLTFAsset.cpp`):
1. Parse `nodes[]` array, extracting TRS or matrix transforms
2. Build parent-child relationships from `children[]`
3. Recursively traverse from scene root nodes
4. For each mesh node: `GlobalTransform = ParentTransform × LocalTransform`
5. Apply transform to Position (w=1), Normal (w=0), Tangent (w=0)
6. Normalize transformed normals/tangents

**Matrix Convention (Critical):**
GLTF stores matrices in **column-major** order. `njMat4` constructor expects **row vectors**. The fix transposes during parsing:
```cpp
// Row 0 = elements 0,4,8,12 (Column 0 of GLTF matrix)
math::njVec4f r0{ matrix[0], matrix[4], matrix[8], matrix[12] };
```

| Pros | Cons |
|:-----|:-----|
| Zero runtime hierarchy cost | Cannot animate individual nodes |
| Works with flat ECS renderer | Instancing requires duplicate bakes |
| Solves Z-up→Y-up permanently | Mesh data grows for multi-instance |

## 3. Implementation History

| Version | Enhancement | Key Change |
|:--------|:------------|:-----------|
| v0.4 | Tangents, Component Types | Basic attribute support |
| v0.5 | Multi-primitive, Materials | `njPrimitive` refactored from triangle to indexed mesh |
| v0.5.1 | Indexed Drawing | `vkCmdDraw` → `vkCmdDrawIndexed`, new `IndexBuffers` class |
| v0.6 | Scene Loader + 1024 Limit | `njSceneLoader`, `MAX_OBJECTS=1024` |
| v0.6.1 | Hierarchy Baking | Node parsing with column→row transpose fix |
| v0.6.2 | Washed-Out Fix | Removed double gamma, rim lighting |
| v0.6.3 | Scale Support | Physics system preserving transforms |

## 4. Next Steps

1. **Animation Runtime** — Requires lazy hierarchy (no baking for animated nodes)
2. **GLTF Cameras** — Parse `cameras[]` array instead of manual JSON
3. **Morph Targets** — For facial animation / blend shapes
