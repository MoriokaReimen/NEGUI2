#ifndef _MEMORY_MANAGER_HPP
#define _MEMORY_MANAGER_HPP
#include <unordered_map>
#include <string>
#include <volk.h>
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc.h>

namespace NEGUI2
{
    struct Memory
    {
        vk::raii::Buffer buffer;
        VmaAllocation alloc;
        VmaAllocationInfo alloc_info;

        enum class TYPE : uint32_t
        {
            VERTEX = 1,
            INDEX = 2,
            UNIFORM = 3
        };
        TYPE type;
    };

    struct Image
    {
        vk::raii::Image image;
        VmaAllocation alloc;
        VmaAllocationInfo alloc_info;
    };

    class MemoryManager
    {
        VmaAllocator allocator_;
        std::unordered_map<std::string, Memory> memories_;
        MemoryManager();
        MemoryManager(const MemoryManager& other) = delete;
        MemoryManager& operator=(const MemoryManager& other) = delete;
    public:
        ~MemoryManager();
        static MemoryManager& getInstacne();
        Memory &get_memory(const std::string &key);
        bool add_memory(const std::string &key, const size_t &size, const Memory::TYPE &type);
        bool remove_memory(const std::string &key);
        bool upload_memory(const std::string &key, const void *data, const size_t size);
    };
}

#endif