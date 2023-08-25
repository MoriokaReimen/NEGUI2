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
        allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
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
            vmaDestroyBuffer(allocator_, *memory.second.buffer, memory.second.alloc);
        }
        vmaDestroyAllocator(allocator_);
    }

    Memory &MemoryManager::get_memory(const std::string &key)
    {
        return memories_.at(key);
    }

    bool MemoryManager::add_memory(const std::string &key, const size_t &size, const Memory::TYPE &type)
    {
        if (memories_.count(key) != 0)
            return false;

        VkBufferCreateInfo bufferInfo{};
        VmaAllocationCreateInfo allocInfo{};

        switch (type)
        {
        case Memory::TYPE::VERTEX:
        {
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = size;
            bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        }
        break;
        case Memory::TYPE::INDEX:
        {
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = size;
            bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        }
        break;
        case Memory::TYPE::UNIFORM:
        {

            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = size;
            bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

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

        auto result = vmaCreateBuffer(allocator_, &bufferInfo, &allocInfo, &buffer, &alloc, &alloc_info);
        auto &device = Core::get_instance().get_device_manager();

        memories_.insert({key, Memory{vk::raii::Buffer(device.device, buffer), alloc, alloc_info}});

        return true;
    }

    bool MemoryManager::remove_memory(const std::string &key)
    {
        bool ret = false;
        if (memories_.count(key) != 0)
        {
            auto &memory = memories_.at(key);
            vmaDestroyBuffer(allocator_, *memory.buffer, memory.alloc);
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
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = size;
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

        /* データコピー */
        {
            std::memcpy(alloc_info.pMappedData, data, size);
            vmaFlushAllocation(allocator_, stage_allocation, 0, VK_WHOLE_SIZE);
            Core::get_instance().get_device_manager().one_shot([&](vk::raii::CommandBuffer &command_buffer)
                                                               {
                    auto &target = memories_.at(key);
                    vk::BufferCopy copyRegion{0, 0, size};
                    command_buffer.copyBuffer(stage_buffer, *target.buffer, copyRegion);
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

    bool MemoryManager::add_image(const std::string &key, const int &width, const int &height, const Image::TYPE &type)
    {
        vk::ImageCreateInfo create_info;
        VmaAllocationCreateInfo allocInfo{};

        switch (type)
        {
        case Image::TYPE::COLOR:
        {
            create_info.imageType = vk::ImageType::e2D;
            create_info.format = vk::Format::eR8G8B8A8Unorm;
            create_info.extent.width = static_cast<uint32_t>(width);
            create_info.extent.height = static_cast<uint32_t>(height);
            create_info.extent.depth = 1;
            create_info.mipLevels = 1;
            create_info.arrayLayers = 1;
            create_info.samples = vk::SampleCountFlagBits::e1;
            create_info.tiling = vk::ImageTiling::eOptimal;
            create_info.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;

            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        }
        break;
        case Image::TYPE::DEPTH:
        {
            auto &physical_device = Core::get_instance().get_device_manager().physical_device;
            create_info.imageType = vk::ImageType::e2D;
            create_info.format = get_depth_format(physical_device);
            create_info.extent.width = static_cast<uint32_t>(width);
            create_info.extent.height = static_cast<uint32_t>(height);
            create_info.extent.depth = 1;
            create_info.mipLevels = 1;
            create_info.arrayLayers = 1;
            create_info.samples = vk::SampleCountFlagBits::e1;
            create_info.tiling = vk::ImageTiling::eOptimal;
            create_info.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
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
        auto temp_info = static_cast<VkImageCreateInfo>(create_info);
        vmaCreateImage(allocator_, &temp_info, &allocInfo, &image, &alloc, &alloc_info);
        auto &device = Core::get_instance().get_device_manager();
        images_.insert({key, Image{vk::raii::Image(device.device, image), create_info.format, alloc, alloc_info, type}});
        
        return true;
    }

    bool MemoryManager::remove_image(const std::string &key)
    {
        bool ret = false;
        if (images_.count(key) != 0)
        {
            auto &image = images_.at(key);
            vmaDestroyImage(allocator_, *image.image, image.alloc);
            memories_.erase(key);
            ret = true;
        }
        return ret;
    }
}