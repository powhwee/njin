# 02-Feb-2026: Washed-Out Rendering Debug Session

## Problem
GLTF models (Stitch character) appeared pale and washed-out compared to reference GLTF viewers.

## Initial Misdiagnosis (Gemini)
Gemini suggested the issue was aggressive tone mapping (`x/(x+0.5)`) and lighting values. This was **partially correct** but missed the primary cause.

## Actual Root Causes

### 1. Double Gamma Correction (Primary)
- **Swapchain** uses `VK_FORMAT_B8G8R8A8_SRGB` which auto-applies gamma correction on output
- **Shader** also manually applied gamma: `pow(result, 1/2.2)`
- Result: Colors gamma-corrected twice, lifting all values and causing pale appearance

**Evidence**: Found in `util.cpp:117` (swapchain format) and `shader.frag:83-85` (manual gamma)

### 2. Rim Lighting (Secondary)
After fixing double gamma, colors were still desaturated with advanced lighting effects. Through elimination testing:
- Removed specular → still broken
- Removed rim lighting → **fixed**

Rim lighting adds white (`vec3(rim)`) on top of the result, which desaturates colors by pushing them toward white.

## Debug Process

| Step | Change | Result |
|------|--------|--------|
| 1 | Remove manual gamma + Reinhard tonemapping | Better but not quite right |
| 2 | Remove tone mapping entirely | Overexposed highlights |
| 3 | Reduce lights + add Reinhard back | Too dark |
| 4 | Balance lights | Still grey/desaturated |
| 5 | Simplify to single light, no tonemapping | **Colors correct** |
| 6 | Add back 3 lights + specular + rim | Problem returned |
| 7 | Remove specular only | Still broken |
| 8 | Remove rim lighting | **Fixed** |
| 9 | Add specular back | Still works |

## Final Solution

```glsl
// shader.frag changes:
// 1. NO manual gamma correction (sRGB swapchain handles it)
// 2. NO tone mapping (balanced lighting doesn't need it)
// 3. NO rim lighting (causes desaturation)
// 4. KEEP: 3 directional lights + Blinn-Phong specular
```

## Key Learnings

1. **sRGB swapchain = no manual gamma**: When using `VK_FORMAT_*_SRGB` swapchain, the GPU handles sRGB encoding. Don't add `pow(x, 1/2.2)` in shader.

2. **Additive white effects desaturate**: Rim lighting, bloom, and similar effects that add white on top of colors will reduce saturation. Use carefully or multiply instead of add.

3. **Tone mapping is optional**: For scenes with controlled lighting that doesn't exceed 1.0, tone mapping just compresses dynamic range unnecessarily. Only needed for HDR content.

4. **Binary search for bugs**: When multiple effects could cause an issue, remove them one at a time to isolate the culprit.

## Files Modified
- `shader/shader.frag` - Removed double gamma, removed rim lighting, simplified lighting model
