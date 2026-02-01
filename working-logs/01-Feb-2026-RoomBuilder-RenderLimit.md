# Working Log - 01 Feb 2026

## Scene Loading Refactor & RoomBuilder Integration

### Overview
Today's session focused on refactoring hardcoded entity creation into a data-driven scene loader and integrating the `RoomBuilder` module. We discovered and fixed a critical rendering limit (`MAX_OBJECTS=16`), enabling high-count entity rendering (verified with 25+ tiles).

### Accomplishments

1.  **Scene Loading Refactor**
    *   Implemented `njSceneLoader` to parse JSON scene files.
    *   Created `main.scene` configuration.
    *   Result: Scenes are now data-driven and configurable.

2.  **RoomBuilder Integration**
    *   Updated `RoomBuilder` to use configured descriptors.
    *   Integrated into `main.cpp` to generate a 5x5 floor grid.
    *   Result: All 25 tiles render correctly after fix.

3.  **Critical Fix: Rendering Limit (v0.6)**
    *   **Issue:** `shader.vert`, `shader.frag`, and `config.h` limited the scene to 16 objects hardcoded (`models[16]`, `textures[16]`, `MAX_OBJECTS=16`).
    *   **Fix:** Increased limit to 1024.
    *   **Compatibility Note:** This uses 1024 descriptors per frame. This works on desktop Windows/Linux but likely exceeds limits on macOS (MoltenVK limit often ~31) or mobile devices.
    *   **Recommendation for Mac:** Refactor to Single SSBO architecture in future release.

### Release Tags

*   **v0.5.3:** Tagged the remote state (before 1024-object changes) as a safe fallback point for Mac compatibility.
*   **v0.6:** Committed and tagged the current state with RoomBuilder and 1024 Object support.
