#ifndef _MEMORY_MANAGER_HPP
#define _MEMORY_MANAGER_HPP
#include <unordered_map>
#include <string>
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc.h>

namespace NEGUI2
{
    struct Memory
    {
        vk::Buffer buffer;
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
        vk::Image image;
        vk::ImageView image_view;
        vk::Sampler sampler;
        vk::Format format;
        VmaAllocation alloc;
        VmaAllocationInfo alloc_info;
        enum class TYPE : uint32_t
        {
            COLOR = 1,
            DEPTH = 2,
            TEXTURE = 3,
        };
        TYPE type;
    };

    class MemoryManager
    {
        friend class Core;
        VmaAllocator allocator_;
        std::unordered_map<std::string, Memory> memories_;
        std::unordered_map<std::string, Image> images_;
        MemoryManager();
        void init();
        MemoryManager(const MemoryManager& other) = delete;
        MemoryManager& operator=(const MemoryManager& other) = delete;
    public:
        ~MemoryManager();
        Memory &get_memory(const std::string &key);
        bool add_memory(const std::string &key, const size_t &size, const Memory::TYPE &type, bool rebuild = true);
        bool remove_memory(const std::string &key);
        bool upload_memory(const std::string &key, const void *data, const size_t size);

        Image &get_image(const std::string &key);
        bool add_image(const std::string &key, const int& width, const int& height, const Image::TYPE &type, bool rebuild = true);
        bool remove_image(const std::string &key);
        bool upload_image(const std::string& key, const void *data, const uint32_t& width, const uint32_t& height);
    };
}

#endif