#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

#include "LogicalDevice.h"
#include "vulkan/PhysicalDevice.h"

namespace njin::vulkan {

    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    };

    /** Settings for creation of the buffer */
    struct BufferSettings {
        VkDeviceSize size;  // size of the buffer to allocate
        VkBufferUsageFlagBits usage{ VK_BUFFER_USAGE_TRANSFER_SRC_BIT };
    };

    class Buffer {
        public:
        Buffer() = default;
        Buffer(const LogicalDevice& device,
               const PhysicalDevice& physical_device,
               const BufferSettings& settings);

        Buffer(const Buffer& other) = delete;
        Buffer& operator=(const Buffer& other) = delete;

        Buffer(Buffer&& other) noexcept;

        Buffer& operator=(Buffer&& other) noexcept;

        ~Buffer();

        // // Load vertex data into buffer
        // template<typename T>
        // void load(const std::vector<T>& vec);
        template<typename T>
        void load(const std::vector<T>& vec) {
            element_count_ = vec.size();

            memcpy(data_, vec.data(), sizeof(T) * element_count_);
        }

        VkBuffer get() const;

        /**
         * Get the number of elements currently loaded into this buffer
         * @return Element count
         */
        size_t get_element_count() const;

        private:
        VkBuffer buffer_{ VK_NULL_HANDLE };
        VkDeviceMemory memory_{ VK_NULL_HANDLE };
        VkDeviceSize memory_size_{ 0 };
        VkDevice device_{ VK_NULL_HANDLE };
        void* data_{ nullptr };  // pointer to memory mapped to gpu buffer

        int element_count_{ 0 };
    };
}  // namespace njin::vulkan
