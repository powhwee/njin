# Architecture Review Report
**Date:** 31 Jan 2026
**Reviewer:** Gemini (Antigravity)

## Executive Summary
The `njin` engine demonstrates a robust, modular architecture with clear separation of concerns. The recent refactoring of the scene loading system (shifting from hardcoded setup to data-driven `main.scene`) was integrated successfully without compromising the architectural integrity.

## Dependency Graph Analysis
The project follows a clean strict hierarchy:

1.  **Apps (`njin`, `test`)**: Top-level composition roots.
2.  **`vulkan`**: Rendering implementation. Depends on `core`, `SDL3`.
3.  **`ecs`**: Logic framework. Depends on `core`, `physics_system`. **Rendering-Agnostic**.
4.  **`core`**: Base types and Data IO.
5.  **`math`, `util`**: Leaf dependencies.

**Status:** ✅ **Healthy**. The move of `njSceneLoader` to `ecs` resolved the potential circular dependency between `core` and `ecs`.

## Module Breakdown

### 1. ECS (Entity Component System)
*   **Purity**: High. The `njRenderSystem` does *not* make Vulkan calls. Instead, it populates a `core::RenderBuffer` with platform-agnostic `Renderable` commands.
*   **Physics**: The `physics_system` is isolated in its own library, depending only on `math`. accurate.
*   **Scene Loading**: `njSceneLoader` now correctly resides here, acting as a factory that translates `core` data into `ecs` archetypes.

### 2. Core (Data & Assets)
*   **Responsibility**: strictly manages data types (`Mesh`, `Texture`, `Material`) and IO.
*   **Asset Lifecycle**: Assets are loaded into `njRegistry` instances in `main.cpp`. These registries are passed as *read-only* (const pointers) to systems, ensuring safety.

### 3. Vulkan (Rendering)
*   **Decoupling**: The renderer consumes the `RenderBuffer` produced by ECS. This theoretically allows swapping the backend (e.g., to DirectX) by replacing only this module.

### 4. MNT (Map and Tile)
*   **Status**: Dormant. `RoomBuilder` is compiled and linked but not currently instantiated in `main.cpp`. It remains available for future procedural generation tasks.

## Code Quality & Technical Debt
*   **C++ Standard**: C++20 features (concepts, ranges) are used effectively.
*   **Memory Management**: Heavy use of `std::unique_ptr` and RAII patterns.
*   **Main Loop**: There is some "game logic" (camera rotation) hardcoded in `main.cpp` (lines 161-168).
    *   *Recommendation*: Move this to a `CameraControlSystem` in the future to fully clear `main.cpp` of loop logic.

## Conclusion
The repository is in excellent shape. The "P1-P4" work is fully integrated and tested. The system is now ready for content expansion via `main.scene` without requiring code recompilation for layout changes.
