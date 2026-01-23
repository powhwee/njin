# Debugging Session: The Case of the Missing Normals (Nov 3, 2025)

## Introduction

This document details a debugging session that took place on November 3, 2025. The initial goal was to understand and improve the rendering quality of the `njin` game engine. The session quickly turned into a deep dive to solve a mysterious rendering bug where lighting effects were not appearing, despite the shader code being seemingly correct. This document serves as a log of the troubleshooting process, particularly the step-by-step investigation that led to the discovery of the root cause.

## Initial Exploration and Setup

Our session began with an exploration of the engine's architecture. We established that `njin` is a modern, ECS-based C++ game engine using Vulkan for rendering.

The first few modifications were straightforward:
1.  **Fill vs. Wireframe:** We changed the rendering mode from wireframe to solid fill by modifying the `polygon_mode` in `njin/vulkan/include/vulkan/config.h`.
2.  **Camera Angle:** We adjusted the camera's position in `njin/main.cpp` to view the rendered object from below.

## The Rendering Mystery Begins: Missing Cavities

The first sign of trouble appeared when we noticed that cavities and dents in the test model, which were visible in wireframe mode, were not appearing in solid fill mode.

My initial hypothesis was that **back-face culling** was enabled, causing the interior faces of the cavities to be discarded. However, upon inspecting `config.h`, we found that `cull_mode` was set to `VK_CULL_MODE_NONE`, ruling this out.

My next hypothesis was that the issue was due to **depth testing**. The outer surfaces of the object were being drawn first, writing to the depth buffer and preventing the farther-away interior surfaces from being rendered. To test this, we changed the `cull_mode` to `VK_CULL_MODE_FRONT_BIT` to render only the back-facing (interior) polygons.

## The Deep Dive: Debugging the Normals

This is where the core of the debugging session began. The user reported that even with front-face culling enabled, the object was still not showing the cavities. This was a critical clue that something more fundamental was wrong.

### Clue 1: The Object is White and Flat

The user pointed out that the object was completely white and lacked any shading, making it hard to perceive its shape. This was because the fragment shader was outputting a hardcoded white color. This led to the decision to implement basic lighting.

### The Plan to Add Lighting

To add lighting, we needed the vertex normals. My plan was:
1.  Correct the vertex input configuration in `config.h` to include normals.
2.  Update the vertex shader to receive and pass on the normals.
3.  Update the fragment shader to calculate diffuse lighting.

I proceeded to "fix" the `VertexInputInfo` in `config.h` to include all the vertex attributes (position, normal, tangent, etc.) and set the correct vertex stride.

### Clue 2: The Object is Black (Normal Visualization)

After implementing the lighting code, the user reported that the object was still not showing any lighting effects. To diagnose this, I proposed we use a classic shader debugging technique: **visualizing the normals as colors**. The fragment shader was modified to output the normal vector as an RGB color.

The user reported that the object was **completely black**. This was a huge clue. It meant that the normal vectors arriving at the fragment shader were likely zero vectors `(0,0,0)`.

### Clue 3: The Object is Red (Zero-Normal Check)

To be absolutely certain that the normals were zero, we performed a more direct test. The fragment shader was modified to output **green** if the normal's length was greater than zero, and **red** otherwise.

The user reported that the object was **red**. This definitively proved that the normals being received by the fragment shader were zero.

### Clue 4: The Object is Grey (Raw Normal Visualization)

We had now isolated the problem to the pipeline *before* the fragment shader. The next step was to determine if the normals were zero at the vertex shader input, or if they were being zeroed out by the transformation in the vertex shader.

To test this, we modified the shaders again:
1.  The vertex shader was changed to pass the **raw, untransformed normal** directly as a color.
2.  The fragment shader was changed to simply output this color.

The user reported that the object was a **single shade of grey**. This was the final, crucial clue. A grey color `(0.5, 0.5, 0.5)` is the result of the expression `normal * 0.5 + 0.5` when the `normal` is `(0,0,0)`.

This proved that the `normal` attribute being read by the vertex shader from the vertex buffer was a zero vector. The problem was not in the shaders, but in the data pipeline between the CPU and the GPU.

### The Final Investigation: Finding the Root Cause

We had now narrowed down the problem to the vertex data itself. I systematically worked my way backward through the rendering pipeline, examining each file responsible for handling the vertex data:
1.  `GraphicsPipeline.cpp`: The code that creates the pipeline state from the `config.h` data. It appeared correct.
2.  `RenderResources.cpp`: The class that owns the vertex buffers.
3.  `VertexBuffers.cpp`: The class that creates the vertex buffers. It correctly allocated the buffers, but did not upload any data to them.
4.  `Buffer.cpp`: The underlying buffer implementation. It correctly mapped the GPU memory for writing, but had no function to perform the write.

The "Aha!" moment came when I realized that the buffers were being created but never filled. The final piece of the puzzle was found in `RenderInfos.cpp`.

**The Bug:** In `RenderInfos.cpp`, the code responsible for preparing the vertex data for rendering was only copying the `x`, `y`, and `z` components of the vertex **position**. All other attributes, including the normals, were being ignored and default-initialized to zero.

**The Fix:** I modified the vertex serialization loop in `RenderInfos.cpp` to copy all the vertex attributes (position, normal, tangent, texture coordinates, and color) into the buffer that gets uploaded to the GPU.

## Lighting and Rendering Improvements

After fixing the bug, the lighting finally worked. We then proceeded through several iterative improvements to enhance the visual quality:
1.  **Specular Highlights:** We implemented a Blinn-Phong lighting model to add highlights.
2.  **Multiple Light Sources:** We increased the number of lights to create more complex lighting and shadows.
3.  **Colored Light and Exposure:** We added a colored light for artistic effect and implemented an exposure control to manage the overall brightness.
4.  **Rim Lighting:** To address the user's feedback that the edges were not well-defined, we added rim lighting to make the object's silhouette "pop".
5.  **Fine-Tuning:** We continued to tweak the exposure and light intensity to achieve the desired look.

## Conclusion

This debugging session was a journey from a simple rendering issue to a deep and subtle bug in the engine's data pipeline. The key takeaway is the power of systematic, step-by-step debugging. By forming a hypothesis, devising a test to prove or disprove it, and carefully observing the results, we were able to progressively narrow down the source of the problem until the root cause was found. The use of shader debugging techniques, like visualizing normals, was instrumental in this process.
