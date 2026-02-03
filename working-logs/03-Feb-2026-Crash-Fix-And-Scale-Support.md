# GLTF Crash Fix & Entity Scale Support - Feb 3, 2026

## Summary
Fixed a critical crash when loading GLTF models with hierarchies, and added data-driven entity scaling support to the scene system.

## Bug 1: STATUS_ACCESS_VIOLATION Crash (Critical)

### Problem
Application crashed with `STATUS_ACCESS_VIOLATION` (0xC0000005) after Frame 1 when loading `dog_puppy.glb` model.

### Root Cause
Null pointer dereference in `njEntityManager::get_view()`. When querying for components like `njParentComponent` that no entity had ever used, `get_component_map()` returned `nullptr`. The function then called `nullptr->get(entity)`, causing the crash.

### Fix
Added null-check to `get_view()` in `njEntityManager.tpp`:
```cpp
// If any component type has never been registered, return a view
// with null pointers for all components
bool any_null{ std::apply(
    [](njComponentMap<Component>*... m) { return ((m == nullptr) || ...); },
    maps) };
if (any_null) {
    std::tuple<Component*...> null_components{ static_cast<Component*>(nullptr)... };
    return { entity, null_components };
}
```

---

## Bug 2: Physics System Discarding Scale (Important)

### Problem
Entity scale specified in `main.scene` had no visible effect. Dog appeared tiny despite `"scale": 35.0`.

### Root Cause
`njPhysicsSystem::calculate_new_transforms()` created a **pure translation matrix** when updating entity positions, discarding the original scale and rotation:
```cpp
// BEFORE: Lost scale/rotation!
math::njMat4f new_transform{ math::njMat4Type::Translation, { new_x, new_y, new_z } };
```

### Fix
Preserve the original transform's rotation/scale, only update translation:
```cpp
// AFTER: Preserves scale/rotation
math::njMat4f new_transform = global_transform;
new_transform[0][3] = new_x;  // Translation X
new_transform[1][3] = new_y;  // Translation Y
new_transform[2][3] = new_z;  // Translation Z
```

---

## Feature: Entity Scale Support

### Implementation
Added `"scale"` property parsing to `njSceneLoader.cpp`:
- **Uniform scale**: `"scale": 6.0` (applies to X, Y, Z equally)
- **Per-axis scale**: `"scale": [2.0, 3.0, 1.0]`

Transform construction order: **Translation × Rotation × Scale**

### Usage
```json
{
  "name": "player",
  "archetype": "player",
  "mesh_alias": "player",
  "position": [0.0, 1.0, 0.0],
  "scale": 6.0,
  "physics": { "mass": 1.0, "type": "dynamic" }
}
```

---

## Files Changed

| File | Change |
|------|--------|
| `njEntityManager.tpp` | Null-check fix for `get_view()` |
| `njPhysicsSystem.cpp` | Preserve scale/rotation in transform updates |
| `njSceneLoader.cpp` | Added scale parsing support |
| `njRenderSystem.cpp` | Cleaned up debug logging |
| `main.scene` | Added `"scale": 6.0` for player |

## Version
- Commit: `29a3b10`
- Tag: `v0.6.3`

## Verification
- ✅ Build successful
- ✅ Application runs without crash
- ✅ Dog model renders at correct scale
- ✅ Pushed to remote with tag
