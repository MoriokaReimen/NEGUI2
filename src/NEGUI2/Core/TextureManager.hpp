#ifndef _TEXTURE_MANAGER_HPP
#define _TEXTURE_MANAGER_HPP
#include <vulkan/vulkan.hpp>
#include <filesystem>
#include <string>
#include <unordered_map>

#include "NEGUI2/Core/MemoryManager.hpp"

namespace NEGUI2
{

    class TextureManager
    {
        friend class Core;
        std::unordered_map<std::string, Image> textures_;
        TextureManager();
        void init(); // TODO すべてのモジュールにデストロイを追加
        TextureManager(const TextureManager& other) = delete;
        TextureManager& operator=(const TextureManager& other) = delete;
    public:
        ~TextureManager();
        Image get(const std::string& key);
        bool load_from_file(const std::filesystem::path& path);
    };
}

#endif