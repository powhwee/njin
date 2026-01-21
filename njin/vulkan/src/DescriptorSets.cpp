#include "vulkan/DescriptorSets.h"

#include "vulkan/DescriptorPool.h"
#include "vulkan/DescriptorSetLayout.h"
#include "vulkan/ImageView.h"

namespace njin::vulkan {
    DescriptorSets::DescriptorSets(const LogicalDevice& device,
                                   const PhysicalDevice& physical_device,
                                   const std::vector<SetLayoutInfo>&
                                   set_layout_infos) :
        device_{ &device },
        physical_device_{ &physical_device } {
        std::vector<DescriptorSetLayout> layouts{};
        for (const SetLayoutInfo& set_layout_info : set_layout_infos) {
            layouts.emplace_back(device, set_layout_info);
        }

        // make the pool
        descriptor_pool_ = { device, set_layout_infos };

        // allocate the descriptor sets from the pool
        std::vector<VkDescriptorSetLayout> vk_set_layouts{};
        for (const DescriptorSetLayout& set_layout : layouts) {
            vk_set_layouts.push_back(set_layout.get());
        }

        uint32_t set_layout_count{
            static_cast<uint32_t>(vk_set_layouts.size())
        };
        VkDescriptorSetAllocateInfo allocate_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = descriptor_pool_.get(),
            .descriptorSetCount = set_layout_count,
            .pSetLayouts = vk_set_layouts.data()
        };

        std::vector<VkDescriptorSet> descriptor_sets(set_layout_count);
        if (vkAllocateDescriptorSets(device_->get(),
                                     &allocate_info,
                                     descriptor_sets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets");
        }

        // store the set layouts into a map
        for (int i{ 0 }; i < set_layout_count; ++i) {
            set_layouts_[set_layout_infos[i].name] = std::move(layouts[i]);
        }

        // store the allocated descriptor sets into the map
        std::vector<std::string> descriptor_set_names{};
        for (const SetLayoutInfo& set_layout_info : set_layout_infos) {
            descriptor_set_names.push_back(set_layout_info.name);
        }
        for (int i{ 0 }; i < set_layout_count; ++i) {
            descriptor_sets_[descriptor_set_names[i]] = descriptor_sets[i];
        }

        // associate the binding names with their index in their set
        uint32_t counter{ 0 };
        for (const SetLayoutInfo& set_layout_info : set_layout_infos) {
            for (const SetLayoutBindingInfo& binding_info :
                 set_layout_info.binding_infos) {
                binding_indices_[{ set_layout_info.name, binding_info.name }] =
                counter;
                ++counter;
            }
            counter = 0;
        }

        create_resources(set_layout_infos);
        write_descriptor_sets(set_layout_infos);
    }

    VkDescriptorSetLayout
    DescriptorSets::get_descriptor_set_layout(const std::string& name) const {
        return set_layouts_.at(name).get();
    }

    VkDescriptorSet
    DescriptorSets::get_descriptor_set(const std::string& name) const {
        return descriptor_sets_.at(name);
    }

    void DescriptorSets::write_descriptor_data(
    const std::string& set_name,
    const std::string& binding_name,
    const std::vector<const core::njTexture*>& data) {
        Resource& resource{ resources_.at({ set_name, binding_name }) };
        auto& images{ std::get<std::vector<ImageSet>>(resource) };

        // if "data" contains n textures then replace the ImageSets at [0...n-1]
        uint32_t descriptor_count{ static_cast<uint32_t>(data.size()) };
        std::vector<VkDescriptorImageInfo>
        descriptor_image_infos(descriptor_count);
        VkWriteDescriptorSet write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = descriptor_sets_[set_name],
            .dstBinding = binding_indices_[{ set_name, binding_name }],
            .dstArrayElement = 0,
            .descriptorCount = descriptor_count,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = descriptor_image_infos.data(),
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        };
        for (uint32_t i{ 0 }; i < descriptor_count; ++i) {
            // load the pixel data into a staging buffer
            const core::njTexture* texture{ data.at(i) };
            BufferSettings buffer_settings{
                .size = texture->get_size_uint64(),
                .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            };
            Buffer image_buffer{ *device_, *physical_device_, buffer_settings };
            image_buffer.load(texture->get_data());

            // create the new resources
            VkExtent2D extent{ texture->get_width(), texture->get_height() };
            VkImageCreateInfo image_create_info{
                .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .imageType = VK_IMAGE_TYPE_2D,
                .format = VK_FORMAT_R8G8B8A8_SRGB,
                .extent = { .width = extent.width,
                            .height = extent.height,
                            .depth = 1 },
                .mipLevels = 1,
                .arrayLayers = 1,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .tiling = VK_IMAGE_TILING_OPTIMAL,
                .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                         VK_IMAGE_USAGE_SAMPLED_BIT,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .queueFamilyIndexCount = 0,
                .pQueueFamilyIndices = nullptr,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            };

            Image image{ *device_, *physical_device_, image_create_info };
            ImageView image_view{ *device_,
                                  image,
                                  extent,
                                  VK_IMAGE_ASPECT_COLOR_BIT };
            // transfer the pixel data into the image
            image.transition_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            image.load_buffer(image_buffer);
            image.transition_layout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            VkDescriptorImageInfo descriptor_image_info{
                .sampler = sampler_.get(),
                .imageView = image_view.get(),
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
            descriptor_image_infos[i] = descriptor_image_info;

            // Replace the existing resource in that array slot
            images[i] = { std::move(image), std::move(image_view) };
        }

        // re-point relevant descriptors to their new images
        vkUpdateDescriptorSets(device_->get(), 1, &write, 0, nullptr);
    }

    void DescriptorSets::write_descriptor_sets(const std::vector<SetLayoutInfo>&
                                               set_layout_infos) {
        // create buffers for bindings that need them (non-images)
        for (const SetLayoutInfo& set_layout_info : set_layout_infos) {
            // work through each set
            for (const SetLayoutBindingInfo& binding_info :
                 set_layout_info.binding_infos) {
                // prepare empty arrays
                std::vector<VkDescriptorBufferInfo>
                buffer_infos(binding_info.descriptor_count);
                std::vector<VkDescriptorImageInfo>
                image_infos(binding_info.descriptor_count);

                // set up common parameters for VkWriteDescriptorSet
                // whether pImageInfo or pBufferInfo or pTexelBufferView is
                // used depends on the descriptor type
                VkWriteDescriptorSet write_info{
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = descriptor_sets_[set_layout_info.name],
                    .dstBinding = binding_info.binding,
                    .dstArrayElement = 0,
                    .descriptorCount = binding_info.descriptor_count,
                    .descriptorType = binding_info.descriptor_type,
                    .pImageInfo = image_infos.data(),
                    .pBufferInfo = buffer_infos.data(),
                    .pTexelBufferView = nullptr
                };

                VkDescriptorType descriptor_type{
                    binding_info.descriptor_type
                };

                if (descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                    descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
                    // populate pBufferInfo
                    auto binding_buffer_info{
                        std::get<SetLayoutBindingBufferInfo>(binding_info
                                                             .extra_info)
                    };
                    const Buffer& buffer{ std::get<Buffer>(
                    resources_[{ set_layout_info.name, binding_info.name }]) };
                    for (int i{ 0 }; i < binding_info.descriptor_count; ++i) {
                        VkDescriptorBufferInfo buffer_info{
                            .buffer = buffer.get(),
                            .offset = i * binding_buffer_info.size,
                            .range = binding_buffer_info.size
                        };
                        buffer_infos[i] = buffer_info;
                    }
                    write_info.pBufferInfo = buffer_infos.data();
                    write_info.pImageInfo = nullptr;
                } else if (descriptor_type ==
                           VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                    // populate pImageInfo
                    SetLayoutBindingImageInfo binding_image_info{
                        std::get<SetLayoutBindingImageInfo>(binding_info
                                                            .extra_info)
                    };
                    // create the VkSampler
                    Sampler sampler{ *device_,
                                     binding_image_info.sampler_info };

                    // dummy images for this binding
                    const auto& images{
                        std::get<std::vector<std::pair<Image, ImageView>>>(
                        resources_[{ set_layout_info.name, binding_info.name }])
                    };
                    for (int i{ 0 }; i < binding_info.descriptor_count; ++i) {
                        // 1 descriptor = 1 image view
                        VkDescriptorImageInfo image_info{
                            .sampler = sampler.get(),
                            .imageView = images[i].second.get(),
                            .imageLayout =
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                        };
                        image_infos[i] = image_info;
                    }
                    write_info.pImageInfo = image_infos.data();
                    write_info.pBufferInfo = nullptr;

                    // keep the sampler alive
                    sampler_ = std::move(sampler);
                }

                vkUpdateDescriptorSets(device_->get(),
                                       1,
                                       &write_info,
                                       0,
                                       nullptr);
            }
        }
    }

    void DescriptorSets::create_resources(const std::vector<SetLayoutInfo>&
                                          set_layout_infos) {
        for (const SetLayoutInfo& set_layout_info : set_layout_infos) {
            for (const SetLayoutBindingInfo& binding_info :
                 set_layout_info.binding_infos) {
                if (binding_info.descriptor_type ==
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                    binding_info.descriptor_type ==
                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
                    // buffers
                    auto binding_buffer_info{
                        std::get<SetLayoutBindingBufferInfo>(binding_info
                                                             .extra_info)
                    };
                    BufferSettings settings{
                        .size = binding_buffer_info.size *
                                binding_info.descriptor_count,
                    };
                    if (binding_info.descriptor_type ==
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                        settings.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
                    } else if (binding_info.descriptor_type ==
                               VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
                        settings.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
                    }
                    Buffer buffer{ *device_, *physical_device_, settings };

                    resources_[{ set_layout_info.name, binding_info.name }] =
                    std::move(buffer);
                } else if (binding_info.descriptor_type ==
                           VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                    // images
                    auto binding_image_info{
                        std::get<SetLayoutBindingImageInfo>(binding_info
                                                            .extra_info)
                    };

                    // Create a dummy image for each descriptor, because
                    // vulkan won't allow descriptors that point to nothing
                    // We can replace these dummy images later
                    VkImageCreateInfo image_create_info{
                        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                        .pNext = nullptr,
                        .flags = 0,
                        .imageType = VK_IMAGE_TYPE_2D,
                        .format = VK_FORMAT_R8G8B8A8_SRGB,
                        .extent = { .width = 1,
                                    .height = 1,
                                    .depth = 1 },  // 1x1 pixel
                        .mipLevels = 1,
                        .arrayLayers = 1,
                        .samples = VK_SAMPLE_COUNT_1_BIT,
                        .tiling = VK_IMAGE_TILING_OPTIMAL,
                        .usage = VK_IMAGE_USAGE_SAMPLED_BIT,
                        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                        .queueFamilyIndexCount = 0,
                        .pQueueFamilyIndices = nullptr,
                        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    };

                    // one image for each descriptor
                    std::vector<std::pair<Image, ImageView>> images{};
                    for (int i{ 0 }; i < binding_info.descriptor_count; ++i) {
                        Image image{ *device_,
                                     *physical_device_,
                                     image_create_info };
                        // Transition dummy image to correct layout
                        image.transition_layout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                        ImageView image_view{ *device_,
                                              image,
                                              { .width = 1, .height = 1 },
                                              VK_IMAGE_ASPECT_COLOR_BIT };
                        images.push_back(std::make_pair(std::move(image),
                                                        std::move(image_view)));
                    }
                    resources_[{ set_layout_info.name, binding_info.name }] =
                    std::move(images);
                }
            }
        }
    }
}  // namespace njin::vulkan
