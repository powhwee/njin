# NJIN Engine Debugging Session (2025-10-28)

This document logs the debugging session for the `njin` Vulkan engine, focusing on resolving a `std::runtime_error` related to descriptor set allocation.

## 1. Initial Analysis

The session began with a request to understand the architecture of the `njin` repository.

**Summary of Architecture:**

*   **Purpose**: A C++ 3D rendering/game engine using the Vulkan API.
*   **Core Modules**:
    *   `njin/vulkan`: Low-level Vulkan rendering backend.
    *   `njin/ecs`: Entity-Component-System for scene logic.
    *   `njin/core`: Core data structures (`Mesh`, `Texture`, etc.).
    *   `njin/util`: Asset loading (glTF) and file utilities.
    *   `njin/math`: Vector and Matrix math library.

## 2. The Error

Upon running the compiled application, it crashed with the following error:

```text
Validation Warning: [ WARNING-VkDescriptorSetAllocateInfo-descriptorCount ] | MessageID = 0x1ed8505a
vkAllocateDescriptorSets(): Trying to allocate 100 of VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER descriptors from VkDescriptorPool 0xf000000000f, but this pool only has a total of 1 descriptors for this type so you will likely get VK_ERROR_OUT_OF_POOL_MEMORY_KHR...

... (similar warnings for UNIFORM_BUFFER and STORAGE_BUFFER) ...

libc++abi: terminating due to uncaught exception of type std::runtime_error: failed to allocate descriptor sets
Abort trap: 6
```

**Diagnosis**: The Vulkan validation layers clearly indicated that the `VkDescriptorPool` was created with insufficient capacity. The code was attempting to allocate descriptor sets that required 100 of certain descriptor types, but the pool was only sized to provide 1 of each.

## 3. Debugging Process

The goal was to find the `VkDescriptorPool` creation logic and correct the size calculation.

### Step 1: Locating the Flaw

The file `njin/vulkan/src/DescriptorPool.cpp` was identified as the location for pool creation. The constructor in question was:

```cpp
// Original flawed code in DescriptorPool.cpp
DescriptorPool::DescriptorPool(const LogicalDevice& device,
                               const std::vector<SetLayoutInfo>& set_layout_infos)
    : device_{ device.get() } {
    std::unordered_map<VkDescriptorType, uint32_t> descriptor_type_to_count;
    for (const SetLayoutInfo& set_layout_info : set_layout_infos) {
        for (const SetLayoutBindingInfo& binding_info :
             set_layout_info.binding_infos) {
            // This was the bug: it ignores array sizes (e.g., 100)
            descriptor_type_to_count[binding_info.descriptor_type]++;
        }
    }
    // ... logic to create pool with incorrect counts ...
}
```
The bug is that `++` increments the count by one, regardless of whether `binding_info.descriptor_count` is 1 or 100.

### Step 2: A Misguided Correction

Initially, I made an incorrect assumption and modified the constructor to accept a `max_sets` parameter, intending to multiply the counts. This led to incorrect changes in both `DescriptorPool.cpp` and its header file, `DescriptorPool.h`. This was a mistake, as the calling code in `DescriptorSets.cpp` was designed to pass all layout information at once, not to allocate multiple sets of the same layout repeatedly.

### Step 3: The Correct Fix

After re-examining `DescriptorSets.cpp`, the correct solution was identified. The `max_sets` parameter was unnecessary. The fix was to simply use the `descriptor_count` from the binding info instead of just incrementing the count.

**Action 1: Revert Header File**

The change to `njin/vulkan/include/vulkan/DescriptorPool.h` was reverted to its original state, removing the `max_sets` parameter from the constructor declaration.

**Action 2: Apply Correct Logic to Source File**

The constructor in `njin/vulkan/src/DescriptorPool.cpp` was corrected as follows:

```cpp
// Corrected code in DescriptorPool.cpp
DescriptorPool::DescriptorPool(const LogicalDevice& device,
                               const std::vector<SetLayoutInfo>& set_layout_infos)
    : device_{ device.get() } {
    std::unordered_map<VkDescriptorType, uint32_t> descriptor_type_to_count;
    for (const SetLayoutInfo& set_layout_info : set_layout_infos) {
        for (const SetLayoutBindingInfo& binding_info :
             set_layout_info.binding_infos) {
            // Correctly sums the total number of descriptors needed for each type
            descriptor_type_to_count[binding_info.descriptor_type] += binding_info.descriptor_count;
        }
    }

    std::vector<VkDescriptorPoolSize> pool_sizes{};
    for (auto it{ descriptor_type_to_count.begin() };
         it != descriptor_type_to_count.end();
         ++it) {
        VkDescriptorPoolSize pool_size{ .type = it->first,
                                        .descriptorCount = it->second };
        pool_sizes.push_back(pool_size);
    }
    // ... rest of the pool creation logic ...
}
```

## 4. Conclusion

The fix correctly calculates the total number of individual descriptors required for the pool by summing the `descriptor_count` from each layout binding. This ensures the `VkDescriptorPool` is created with the correct capacity, resolving the `VK_ERROR_OUT_OF_POOL_MEMORY_KHR` error and the subsequent application crash. The application should now compile and run successfully.
