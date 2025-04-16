#include "vulkan/Buffer.h"

#include <stdexcept>
#include <vector>

#include <vulkan/DescriptorSetLayoutBuilder.h>
#include <vulkan/util.h>

#include "vulkan/Renderer.h"

namespace {
    using namespace njin::vulkan;
}  // namespace

namespace njin::vulkan {
    Buffer::Buffer(const LogicalDevice& device,
                   const PhysicalDevice& physical_device,
                   const BufferSettings& settings) :
        device_{ device.get() } {
        VkBufferCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.size = settings.size;
        info.usage = settings.usage;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // graphics queue only
        info.queueFamilyIndexCount = 0;
        info.pQueueFamilyIndices = nullptr;

        // create the buffer. Note that the buffer is just a conceptual object
        // and doesn't have memory. It just acts as a handle to memory
        if (vkCreateBuffer(device_, &info, nullptr, &buffer_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create vertex buffer");
        }

        VkMemoryRequirements memory_requirements{};
        vkGetBufferMemoryRequirements(device_, buffer_, &memory_requirements);
        memory_size_ = memory_requirements.size;

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = memory_size_;
        alloc_info.memoryTypeIndex =
        find_memory_type(physical_device,
                         memory_requirements.memoryTypeBits,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        // allocate actual memory on the device and bind to the buffer object
        if (vkAllocateMemory(device_, &alloc_info, nullptr, &memory_) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory");
        }

        // bind memory to buffer
        vkBindBufferMemory(device_, buffer_, memory_, 0);

        // persistent map to buffer
        vkMapMemory(device_, memory_, 0, memory_size_, 0, &data_);
    }

    Buffer::Buffer(Buffer&& other) noexcept :
        buffer_{ other.buffer_ },
        memory_{ other.memory_ },
        memory_size_{ other.memory_size_ },
        device_{ other.device_ },
        data_{ other.data_ },
        element_count_{ other.element_count_ } {
        other = Buffer();
    }

    Buffer& Buffer::operator=(Buffer&& other) noexcept {
        buffer_ = other.buffer_;
        memory_ = other.memory_;
        memory_size_ = other.memory_size_;
        device_ = other.device_;
        data_ = other.data_;
        element_count_ = other.element_count_;

        other.buffer_ = VK_NULL_HANDLE;
        other.memory_ = VK_NULL_HANDLE;
        other.memory_size_ = 0;
        other.device_ = VK_NULL_HANDLE;
        other.data_ = nullptr;
        other.element_count_ = 0;

        return *this;
    }

    Buffer::~Buffer() {
        if (device_ != VK_NULL_HANDLE) {
            vkDestroyBuffer(device_, buffer_, nullptr);
            vkFreeMemory(device_, memory_, nullptr);
        }
    }

    VkBuffer Buffer::get() const {
        return buffer_;
    }

    size_t Buffer::get_element_count() const {
        return element_count_;
    }
}  // namespace njin::vulkan
