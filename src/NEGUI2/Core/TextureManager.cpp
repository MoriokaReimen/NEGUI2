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
        auto &core = Core::get_instance();
        vk::Device device = *core.get_device_manager().device;

        for(auto& texture : textures_)
        {
            remove(texture.first);
        }
    }

    void TextureManager::init()
    {
        textures_.clear();
    }

    Texture TextureManager::get(const std::string &key)
    {
        return textures_.at(key);
    }

    Texture TextureManager::load_from_file(const std::filesystem::path &path)
    {
        auto abs_path = std::filesystem::absolute(path);
        int width, height, channels;
        constexpr int DESIRED_CHANNELS = 4;
        stbi_uc *img = stbi_load(abs_path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        std::size_t img_size = width * height * DESIRED_CHANNELS;

        Texture texture;
        if (img != nullptr)
        {
            auto &memory_manager = Core::get_instance().get_memory_manager();
            memory_manager.add_image(path.string(), width, height, Image::TYPE::TEXTURE, true);
            Image image = memory_manager.get_image(path.string());
            texture.image = image.image;
            texture.format = image.format;
            texture.alloc = image.alloc;
            texture.alloc_info = image.alloc_info;

            /* イメージビュー作成 */
            vk::ImageView view;
            vk::Sampler sampler;

            auto& core = Core::get_instance();
            vk::Device device = *core.get_device_manager().device;
            {
                vk::ImageSubresourceRange range;
                range.setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setBaseMipLevel(0)
                    .setLevelCount(1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1);

                vk::ImageViewCreateInfo create_info;
                create_info.setImage(image.image)
                    .setViewType(vk::ImageViewType::e2D)
                    .setFormat(image.format)
                    .setSubresourceRange(range);
                texture.image_view = device.createImageView(create_info);
            }

            {
                vk::SamplerCreateInfo create_info;
                create_info.setMagFilter(vk::Filter::eLinear)
                    .setMinFilter(vk::Filter::eLinear)
                    .setAddressModeU(vk::SamplerAddressMode::eRepeat)
                    .setAddressModeV(vk::SamplerAddressMode::eRepeat)
                    .setAddressModeW(vk::SamplerAddressMode::eRepeat)
                    .setAnisotropyEnable(vk::False) // TODO 有効化
                    .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
                    .setUnnormalizedCoordinates(vk::False)
                    .setCompareEnable(vk::False)
                    .setCompareOp(vk::CompareOp::eAlways)
                    .setMipmapMode(vk::SamplerMipmapMode::eLinear)
                    .setMipLodBias(0.f)
                    .setMinLod(0.f)
                    .setMaxLod(0.f);
                
                texture.sampler = device.createSampler(create_info);
            }

            textures_.insert({path.string(), texture});
            memory_manager.upload_image(path.string(), img, width, height);
        }
        else
        {
            spdlog::error("Failed to load {}", abs_path.string());
        }
        stbi_image_free(img);

        return texture;
    }

    // TODO remove処理を追加
    bool TextureManager::remove(const std::string& key)
    {
        bool ret = false;
        if(textures_.count(key) != 0)
        {
            ret = true;
            auto texture = textures_.at(key);
            auto &core = Core::get_instance();
            vk::Device device = *core.get_device_manager().device;

            device.destroyImageView(texture.image_view);
            device.destroySampler(texture.sampler);
        }

        return ret;
    }
    // TODO リファクタリング
}