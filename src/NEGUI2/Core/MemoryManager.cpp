#include "NEGUI2/Core/MemoryManager.hpp"
#include "NEGUI2/Core/Core.hpp"
#include <spdlog/spdlog.h>
#include <exception>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace
{
    vk::Format get_color_format()
    {
        return vk::Format::eR8G8B8A8Unorm;
    }

    bool hasStencilComponent(vk::Format format)
    {
        return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
    }

    vk::Format get_depth_format(const vk::raii::PhysicalDevice &physical_device)
    {
        std::vector<vk::Format> candidates = {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint};
        for (vk::Format format : candidates)
        {
            vk::FormatProperties props = physical_device.getFormatProperties(format);

            if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
            {
                return format;
            }
        }
        spdlog::error("Nodepth Format Found");
        throw std::runtime_error("failed to find supported format!");
    }
}
namespace NEGUI2
{
    MemoryManager::MemoryManager()
    {
    }

    void MemoryManager::init()
    {
        spdlog::info("Initialize Memory Manager.");

        VmaVulkanFunctions fn;
        fn.vkAllocateMemory = vkAllocateMemory;
        fn.vkBindBufferMemory = vkBindBufferMemory;
        fn.vkBindImageMemory = vkBindImageMemory;
        fn.vkCmdCopyBuffer = vkCmdCopyBuffer;
        fn.vkCreateBuffer = vkCreateBuffer;
        fn.vkCreateImage = vkCreateImage;
        fn.vkDestroyBuffer = vkDestroyBuffer;
        fn.vkDestroyImage = vkDestroyImage;
        fn.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
        fn.vkFreeMemory = vkFreeMemory;
        fn.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
        fn.vkGetBufferMemoryRequirements2KHR = (PFN_vkGetBufferMemoryRequirements2KHR)vkGetBufferMemoryRequirements2;
        fn.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
        fn.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
        fn.vkGetImageMemoryRequirements2KHR = (PFN_vkGetImageMemoryRequirements2KHR)vkGetImageMemoryRequirements2;
        fn.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        fn.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
        fn.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
        fn.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
        fn.vkMapMemory = vkMapMemory;
        fn.vkUnmapMemory = vkUnmapMemory;
        fn.vkBindImageMemory2KHR = vkBindImageMemory2;
        fn.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;

        auto &device_manager = Core::get_instance().get_device_manager();
        VmaAllocatorCreateInfo allocatorCreateInfo = {};
        allocatorCreateInfo.vulkanApiVersion = vk::ApiVersion12;
        allocatorCreateInfo.instance = *device_manager.instance;
        allocatorCreateInfo.physicalDevice = *device_manager.physical_device;
        allocatorCreateInfo.device = *device_manager.device;
        allocatorCreateInfo.pVulkanFunctions = &fn;
        vmaCreateAllocator(&allocatorCreateInfo, &allocator_);
    }

    MemoryManager::~MemoryManager()
    {
        for (auto &memory : memories_)
        {
            vmaDestroyBuffer(allocator_, memory.second.buffer, memory.second.alloc);
        }

        for (auto &image : images_)
        {
            auto &core = Core::get_instance();
            auto dm = *core.get_device_manager().device;
#if 0
            dm.destroyImageView(image.second.image_view); // TODO TextureManagerに移動
            dm.destroySampler(image.second.sampler);
#endif
            vmaDestroyImage(allocator_, image.second.image, image.second.alloc);
        }
        vmaDestroyAllocator(allocator_);
    }

    Memory &MemoryManager::get_memory(const std::string &key)
    {
        return memories_.at(key);
    }

    bool MemoryManager::add_memory(const std::string &key, const size_t &size, const Memory::TYPE &type, bool rebuild)
    {
        if (memories_.count(key) != 0 && !rebuild)
            return false;

        if (rebuild)
        {
            remove_memory(key);
        }


        vk::BufferCreateInfo buffer_info;
        vk::BufferViewCreateInfo view_info;
        VmaAllocationCreateInfo allocInfo{};

        switch (type)
        {
        case Memory::TYPE::VERTEX:
        {
            buffer_info.setSize(size)
                       .setUsage(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer)
                       .setSharingMode(vk::SharingMode::eExclusive);
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        }
        break;
        case Memory::TYPE::INDEX:
        {
            buffer_info.setSize(size)
                       .setUsage(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer)
                       .setSharingMode(vk::SharingMode::eExclusive);
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        }
        break;
        case Memory::TYPE::UNIFORM:
        {
            buffer_info.setSize(size)
                       .setUsage(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer)
                       .setSharingMode(vk::SharingMode::eExclusive);

            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                              VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }
        break;
        default:
            return false;
            break;
        }
        VkBuffer buffer;
        VmaAllocation alloc;
        VmaAllocationInfo alloc_info;

        auto result = vmaCreateBuffer(allocator_, reinterpret_cast<const VkBufferCreateInfo*>(&buffer_info), &allocInfo, &buffer, &alloc, &alloc_info);
        auto &device = Core::get_instance().get_device_manager();

        memories_.insert({key, Memory{vk::Buffer(buffer), alloc, alloc_info}});

        return true;
    }

    bool MemoryManager::remove_memory(const std::string &key)
    {
        bool ret = false;
        if (memories_.count(key) != 0)
        {
            auto &memory = memories_.at(key);
            vmaDestroyBuffer(allocator_, memory.buffer, memory.alloc);
            memories_.erase(key);
            ret = true;
        }
        return ret;
    }

    bool MemoryManager::upload_memory(const std::string &key, const void *data, const size_t size)
    {
        if (memories_.count(key) == 0 || size == 0)
        {
            return false;
        }

        /* ステージングバッファ生成 */
        VkBuffer stage_buffer;
        VmaAllocation stage_allocation;
        VmaAllocationInfo alloc_info;
        {
            vk::BufferCreateInfo buffer_info;
            buffer_info.setSize(size)
                       .setUsage(vk::BufferUsageFlagBits::eTransferSrc)
                       .setSharingMode(vk::SharingMode::eExclusive);

            VmaAllocationCreateInfo allocInfo{};
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                              VMA_ALLOCATION_CREATE_MAPPED_BIT;
            VkBuffer buffer;
            VmaAllocation allocation;
            vmaCreateBuffer(allocator_, reinterpret_cast<const VkBufferCreateInfo*>(&buffer_info), &allocInfo, &stage_buffer, &stage_allocation, &alloc_info);
        }

        /* データコピー */
        {
            std::memcpy(alloc_info.pMappedData, data, size);
            vmaFlushAllocation(allocator_, stage_allocation, 0, VK_WHOLE_SIZE);
            Core::get_instance().get_device_manager().one_shot([&](vk::raii::CommandBuffer &command_buffer)
                                                               {
                    auto &target = memories_.at(key);
                    vk::BufferCopy copyRegion{0, 0, size};
                    command_buffer.copyBuffer(stage_buffer, target.buffer, copyRegion);
                    return vk::Result::eSuccess; });
        }

        /* ステージバッファ削除 */
        vmaDestroyBuffer(allocator_, stage_buffer, stage_allocation);

        return true;
    }

    Image &MemoryManager::get_image(const std::string &key)
    {
        return images_.at(key);
    }

    bool MemoryManager::add_image(const std::string &key, const int &width, const int &height, const Image::TYPE &type, bool rebuild)
    {
        if (images_.count(key) && !rebuild) // Imageを返す
            return false;

        if (rebuild)
        {
            remove_image(key);
        }

        vk::ImageCreateInfo image_create_info;
        VmaAllocationCreateInfo allocInfo{};

        switch (type)
        {
        case Image::TYPE::COLOR:
        {
            image_create_info.setImageType(vk::ImageType::e2D)
                             .setFormat(vk::Format::eR8G8B8A8Unorm)
                             .setExtent({static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1u})
                             .setMipLevels(1)
                             .setArrayLayers(1)
                             .setSamples(vk::SampleCountFlagBits::e1)
                             .setTiling(vk::ImageTiling::eOptimal)
                             .setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        }
        break;
        case Image::TYPE::DEPTH:
        {
            auto &physical_device = Core::get_instance().get_device_manager().physical_device;
            image_create_info.imageType = vk::ImageType::e2D;
            image_create_info.format = get_depth_format(physical_device);
            image_create_info.extent.width = static_cast<uint32_t>(width);
            image_create_info.extent.height = static_cast<uint32_t>(height);
            image_create_info.extent.depth = 1;
            image_create_info.mipLevels = 1;
            image_create_info.arrayLayers = 1;
            image_create_info.samples = vk::SampleCountFlagBits::e1;
            image_create_info.tiling = vk::ImageTiling::eOptimal;
            image_create_info.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        }
        break;
        case Image::TYPE::TEXTURE:
        {
            image_create_info.imageType = vk::ImageType::e2D;
            image_create_info.format = vk::Format::eR8G8B8A8Srgb;
            image_create_info.extent.width = static_cast<uint32_t>(width);
            image_create_info.extent.height = static_cast<uint32_t>(height);
            image_create_info.extent.depth = 1;
            image_create_info.mipLevels = 1;
            image_create_info.arrayLayers = 1;
            image_create_info.initialLayout = vk::ImageLayout::eUndefined;
            image_create_info.samples = vk::SampleCountFlagBits::e1;
            image_create_info.tiling = vk::ImageTiling::eOptimal;
            image_create_info.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
            image_create_info.sharingMode = vk::SharingMode::eExclusive;
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        }
        break;

        default:
            spdlog::error("Invalid image type");
            break;
        }
        VkImage image;
        VmaAllocation alloc;
        VmaAllocationInfo alloc_info; // TODO 改名
        vmaCreateImage(allocator_, reinterpret_cast<const VkImageCreateInfo*>(&image_create_info), &allocInfo, &image, &alloc, &alloc_info);
        auto device = *Core::get_instance().get_device_manager().device;
         images_.insert({key, Image{vk::Image(image), image_create_info.format, alloc, alloc_info, type}});

        return true;
    }

    bool MemoryManager::remove_image(const std::string &key)
    {
        bool ret = false;
        if (images_.count(key) != 0)
        {
            auto &image = images_.at(key);
            vmaDestroyImage(allocator_, image.image, image.alloc);
            images_.erase(key);
            ret = true;
        }
        return ret;
    }

    bool MemoryManager::upload_image(const std::string &key, const void *data, const uint32_t &width, const uint32_t &height)
    {
        if (images_.count(key) == 0 || width * height == 0)
        {
            return false;
        }

        /* イメージサイズを計算 */
        constexpr uint32_t CHANNELS = 4u;
        const uint32_t image_size = CHANNELS * width * height;

        /* ステージングバッファ生成 */
        VkBuffer stage_buffer;
        VmaAllocation stage_allocation;
        VmaAllocationInfo alloc_info;
        {
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = image_size;
            bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo allocInfo{};
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                              VMA_ALLOCATION_CREATE_MAPPED_BIT;
            VkBuffer buffer;
            VmaAllocation allocation;
            vmaCreateBuffer(allocator_, &bufferInfo, &allocInfo, &stage_buffer, &stage_allocation, &alloc_info);
        }

        /* ステージにデータコピー */
        {
            std::memcpy(alloc_info.pMappedData, data, image_size);
            vmaFlushAllocation(allocator_, stage_allocation, 0, VK_WHOLE_SIZE);
            Core::get_instance().get_device_manager().one_shot([&](vk::raii::CommandBuffer &command_buffer)
                                                               {
                    auto &target = images_.at(key);
                    vk::ImageSubresourceLayers subresource;
                    subresource.setAspectMask(vk::ImageAspectFlagBits::eColor)
                               .setMipLevel(0)
                               .setBaseArrayLayer(0)
                               .setLayerCount(1);
                    vk::BufferImageCopy copy_region{0, width, height, subresource, {}, {width, height, 1}};
                    std::array<vk::BufferImageCopy, 1> copyRegions{copy_region};
                    
                    /* データ変換 */
                    {
                        vk::ImageSubresourceRange subresource;
                        subresource.setAspectMask(vk::ImageAspectFlagBits::eColor)
                                   .setBaseMipLevel(0)
                                   .setLevelCount(1)
                                   .setBaseArrayLayer(0)
                                   .setLayerCount(1);

                        vk::ImageMemoryBarrier transfer_barrier;
                        transfer_barrier.setOldLayout(vk::ImageLayout::eUndefined)
                                        .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                                        .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
                                        .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
                                        .setImage(target.image)
                                        .setSubresourceRange(subresource)
                                        .setSrcAccessMask(vk::AccessFlagBits::eNone)
                                        .setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
                        
                        command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                                       vk::PipelineStageFlagBits::eTransfer,
                                                       {},
                                                       {},
                                                       {},
                                                       {transfer_barrier});
                    }

                    /* デバイスメモリ間コピー */
                    command_buffer.copyBufferToImage(stage_buffer, target.image, vk::ImageLayout::eTransferDstOptimal, copyRegions);

                    /* データ変換 */
                    {
                        vk::ImageSubresourceRange subresource;
                        subresource.setAspectMask(vk::ImageAspectFlagBits::eColor)
                                   .setBaseMipLevel(0)
                                   .setLevelCount(1)
                                   .setBaseArrayLayer(0)
                                   .setLayerCount(1);

                        vk::ImageMemoryBarrier transfer_barrier;
                        transfer_barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                                        .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                                        .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
                                        .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
                                        .setImage(target.image)
                                        .setSubresourceRange(subresource)
                                        .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                                        .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
                        
                        command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                                       vk::PipelineStageFlagBits::eFragmentShader,
                                                       {},
                                                       {},
                                                       {},
                                                       {transfer_barrier});
                    }
                    
                    return vk::Result::eSuccess; });
        }

        /* ステージバッファ削除 */
        vmaDestroyBuffer(allocator_, stage_buffer, stage_allocation);

        return true;
    }
}