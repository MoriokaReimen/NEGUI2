#include "NEGUI2/Core/MemoryManager.hpp"
#include "NEGUI2/Core/DeviceManager.hpp"

namespace NEGUI2
{
    MemoryManager::MemoryManager()
    {
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

        auto &device_data = DeviceManager::getInstance();
        VmaAllocatorCreateInfo allocatorCreateInfo = {};
        allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
        allocatorCreateInfo.instance = *device_data.instance;
        allocatorCreateInfo.physicalDevice = *device_data.physical_device;
        allocatorCreateInfo.device = *device_data.device;
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

    MemoryManager &MemoryManager::getInstacne()
    {
        static MemoryManager instance;

        return instance;
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
        auto &device = DeviceManager::getInstance();

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
            DeviceManager::getInstance().one_shot([&](vk::raii::CommandBuffer &command_buffer)
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
}