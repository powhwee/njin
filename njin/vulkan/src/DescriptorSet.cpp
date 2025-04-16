#include "vulkan/DescriptorSet.h"

#include <vulkan/Buffer.h>
#include <vulkan/DescriptorSetLayoutBuilder.h>

namespace njin::vulkan {

    DescriptorSet::DescriptorSet(const LogicalDevice& logical_device,
                                 const PhysicalDevice& physical_device,
                                 VkDescriptorSet descriptor_set,
                                 const DescriptorSetLayout& layout) :
        descriptor_set_{ descriptor_set } {
        auto binding_map{ layout.get_binding_map() };
        for (auto [name, binding] : binding_map) {
            // make one persistently mapped buffer for each binding
            // we have in the layout
            if (binding.descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                BufferSettings settings{
                    .size = 65536,
                    .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
                };

                // buffer for bufferinfo descriptors in this binding
                Buffer buffer{ logical_device, physical_device, settings };

                // 1 buffer info for each descriptor
                // each buffer info references a different section of the
                // buffer
                std::vector<VkDescriptorBufferInfo> buffer_infos{};
                for (int i{ 0 }; i < binding.descriptor_count; ++i) {
                    auto extra{ std::get<DescriptorSetBindingBufferSettings>(
                    binding.extra_settings) };
                    // associate this buffer with this binding
                    VkDescriptorBufferInfo buffer_info{
                        .buffer = buffer.get(),
                        .offset = extra.descriptor_size * i,
                        .range = extra.descriptor_size
                    };
                    buffer_infos.push_back(buffer_info);
                }

                VkWriteDescriptorSet write{
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = descriptor_set,
                    .dstBinding = binding.binding,
                    .dstArrayElement = 0,
                    .descriptorCount = binding.descriptor_count,
                    .descriptorType = binding.descriptor_type,
                    .pImageInfo = nullptr,
                    .pBufferInfo = buffer_infos.data(),
                    .pTexelBufferView = nullptr
                };

                vkUpdateDescriptorSets(logical_device.get(),
                                       1,
                                       &write,
                                       0,
                                       nullptr);

                buffers_.insert({ name, std::move(buffer) });
            } else if (binding.descriptor_type ==
                       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                std::vector<VkDescriptorImageInfo> image_infos{};
                for (int i{ 0 }; i < binding.descriptor_count; ++i) {
                    auto extra{ std::get<DescriptorSetBindingImageSetting>(
                    binding.extra_settings) };
                    VkDescriptorImageInfo image_info{
                        .sampler = extra.sampler,
                        .imageView = extra.image_views.at(i),
                        .imageLayout = extra.image_layout
                    };
                    image_infos.push_back(image_info);
                }

                VkWriteDescriptorSet write{
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = descriptor_set,
                    .dstBinding = binding.binding,
                    .dstArrayElement = 0,
                    .descriptorCount = binding.descriptor_count,
                    .descriptorType = binding.descriptor_type,
                    .pImageInfo = image_infos.data(),
                    .pBufferInfo = nullptr,
                    .pTexelBufferView = nullptr
                };

                vkUpdateDescriptorSets(logical_device.get(),
                                       1,
                                       &write,
                                       0,
                                       nullptr);
            } else if (binding.descriptor_type ==
                       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
                BufferSettings settings{
                    .size = 65536,
                    .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
                };

                Buffer buffer{ logical_device, physical_device, settings };

                // 1 buffer info for each descriptor
                // each buffer info references a different section of the
                // buffer
                std::vector<VkDescriptorBufferInfo> buffer_infos{};
                for (int i{ 0 }; i < binding.descriptor_count; ++i) {
                    auto extra{ std::get<DescriptorSetBindingBufferSettings>(
                    binding.extra_settings) };
                    // associate this buffer with this binding
                    VkDescriptorBufferInfo buffer_info{
                        .buffer = buffer.get(),
                        .offset = extra.descriptor_size * i,
                        .range = extra.descriptor_size
                    };
                    buffer_infos.push_back(buffer_info);
                }

                VkWriteDescriptorSet write{
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = descriptor_set,
                    .dstBinding = binding.binding,
                    .dstArrayElement = 0,
                    .descriptorCount = binding.descriptor_count,
                    .descriptorType = binding.descriptor_type,
                    .pImageInfo = nullptr,
                    .pBufferInfo = buffer_infos.data(),
                    .pTexelBufferView = nullptr
                };

                vkUpdateDescriptorSets(logical_device.get(),
                                       1,
                                       &write,
                                       0,
                                       nullptr);

                buffers_.insert({ name, std::move(buffer) });
            }
        }
    }

    VkDescriptorSet DescriptorSet::get() const {
        return descriptor_set_;
    }

    DescriptorSet::DescriptorSet(DescriptorSet&& other) noexcept :
        descriptor_set_{ other.descriptor_set_ },
        buffers_{ std::move(other.buffers_) } {
        other = DescriptorSet();
    }

    DescriptorSet& DescriptorSet::operator=(DescriptorSet&& other) noexcept {
        descriptor_set_ = other.descriptor_set_;
        buffers_ = std::move(other.buffers_);
        other.descriptor_set_ = nullptr;
        other.buffers_.clear();
        return *this;
    }

    template<typename T>
    void DescriptorSet::update_descriptor_data(const std::string& name,
                                               const std::vector<T>& new_data) {
        if (!buffers_.contains(name)) {
            throw std::runtime_error("Descriptor set binding with name " +
                                     name + " not found");
        }
        Buffer& buffer{ buffers_.at(name) };
        buffer.load(new_data);
    }

    template void DescriptorSet::update_descriptor_data<math::njMat4f>(
    const std::string& name,
    const std::vector<math::njMat4f>& new_data);
}  // namespace njin::vulkan