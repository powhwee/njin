# Prioritized List of glTF Loader Enhancements (Simplest to Most Complex)

This document outlines the missing features in the current glTF loader, ordered by the estimated complexity of implementation.

### 1. Incomplete Attribute Support (Tangents)
*   **Why it's simple:** The code for loading tangents is already written but commented out. It follows the exact same pattern as loading positions and normals. Enabling it would likely just involve uncommenting the code and adding a `tangent_attributes_` member to the class.

### 2. Limited Index/Component Types
*   **Why it's simple:** This involves adding a switch-case or if-else block when reading indices or attributes to handle different component types (e.g., `unsigned int` for 32-bit indices in addition to the existing `unsigned short`). The logic is similar to the existing `get_component_type` function but would need to be applied during data retrieval.

### 3. Multiple Primitives and Meshes
*   **Why it's moderately complex:** Instead of accessing `meshes[0].primitives[0]`, you would need to loop through the `meshes` array and, within that, loop through the `primitives` array for each mesh. This requires changing the class structure from representing a single mesh to holding a list of meshes, each with its own geometry data. The file parsing logic itself doesn't change much, but the data storage design does.

### 4. Materials and Textures
*   **Why it's complex:** This is a multi-step process. You need to:
    1.  Parse the `materials`, `textures`, `images`, and `samplers` blocks from the JSON.
    2.  Load image data (e.g., PNG, JPG) from the binary buffer or external files. This may require adding an image-loading library (like `stb_image`).
    3.  Create data structures to represent these concepts.
    4.  Associate the loaded materials with the correct mesh primitives, as defined in the glTF.

### 5. Scene Hierarchy and Nodes
*   **Why it's complex:** This is a fundamental change. It requires parsing the `nodes` array and building a scene graph (a tree structure). Each node has a transformation (matrix or translation/rotation/scale), and can have children nodes and a mesh attached. This introduces the need for a `Node` or `GameObject` class and requires traversing the hierarchy to compute the correct world-space transformations for every object.

### 6. Cameras
*   **Why it's complex:** Similar to the scene hierarchy, this involves parsing the `camera` definitions and attaching them to nodes in the scene graph. It requires creating a `Camera` class with properties like field-of-view, aspect ratio, and projection type (perspective/orthographic).

### 7. Animations and Skinning
*   **Why it's the most complex:** This is by far the most difficult feature to implement.
    *   **Animations**: Requires parsing `animation` data, which includes keyframes for translation, rotation, and scale. You need an animation system that can interpolate between these keyframes over time and apply the resulting transformations to the correct nodes.
    *   **Skinning**: For skeletal animation, you must parse `skins`, `joints`, and inverse bind matrices. This involves implementing a skinning system on the CPU or GPU (via shaders) to deform the mesh vertices based on the pose of a skeleton (a hierarchy of joints). This is a significant architectural addition.
