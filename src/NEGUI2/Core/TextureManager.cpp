#include "NEGUI2/Core/TextureManager.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <spdlog/spdlog.h>

#include "NEGUI2/Core/Core.hpp"

namespace NEGUI2
{
        TextureManager::TextureManager()
        {

        }

        TextureManager::~TextureManager()
        {
            
        }

        void TextureManager::init()
        {
            textures_.clear();
        }

        Image TextureManager::get(const std::string& key)
        {
            return textures_.at(key);
        }

        bool TextureManager::load_from_file(const std::filesystem::path& path)
        {
            bool ret = false;
            auto abs_path = std::filesystem::absolute(path);
            int width, height, channels;
            constexpr int DESIRED_CHANNELS = 4;
            stbi_uc* img = stbi_load(abs_path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            std::size_t img_size = width * height * DESIRED_CHANNELS;
            if(img != nullptr)
            {
                auto &memory_manager = Core::get_instance().get_memory_manager();
                memory_manager.add_image(path.string(), width, height, Image::TYPE::TEXTURE, true);
                Image texture = memory_manager.get_image(path.string());
                textures_.insert({path.string(), texture});
                memory_manager.upload_image(path.string(), img, width, height);
            } else {
                spdlog::error("Failed to load {}", abs_path.string());
            }
            stbi_image_free(img);

            return ret;
        }
}