#ifndef _TEXTURE_MANAGER_HPP
#define _TEXTURE_MANAGER_HPP
#include <vulkan/vulkan.hpp>
#include <filesystem>
#include <string>
#include <unordered_map>

#include "NEGUI2/Core/MemoryManager.hpp"

namespace NEGUI2
{

    struct Texture
    {
        vk::Image image;
        vk::ImageView image_view;
        vk::Sampler sampler;
        vk::Format format;
        VmaAllocation alloc;
        VmaAllocationInfo alloc_info;
    };

    class TextureManager
    {
        friend class Core;
        std::unordered_map<std::string, Texture> textures_;
        TextureManager();
        void init(); // TODO すべてのモジュールにデストロイを追加
        TextureManager(const TextureManager& other) = delete;
        TextureManager& operator=(const TextureManager& other) = delete;
    public:
        ~TextureManager();
        Texture get(const std::string& key);
        Texture load_from_file(const std::filesystem::path& path);
        bool remove(const std::string& key);
    };
}

#endif