#include "NEGUI2/Core/OffScreenManager.hpp"
#include "NEGUI2/Core/Core.hpp"
#include <spdlog/spdlog.h>

namespace NEGUI2
{
    OffScreenManager::OffScreenManager() : extent{1920u, 1080u},
                                           render_pass(nullptr),
                                           sampler(nullptr),
                                           clear_value(), swap_chain_rebuild(false),
                                           frame()

    {
    }

    void OffScreenManager::init()
    {
        auto &window = Core::get_instance().window;
        auto &device_manager = Core::get_instance().gpu;

        /* 背景色設定 */
        {
            clear_value[0].setColor({200.0f / 256.0f, 200.0f / 256.0f, 200.0f / 256.0f, 1.0f});
            clear_value[1].setDepthStencil({1.f, 1u});
            clear_value[2].setColor({0.f, 0.f, 0.f, 0.f});
        }
        rebuild();
    }

    void OffScreenManager::rebuild()
    {
        auto &device_manager = Core::get_instance().gpu;
        auto &memory_manager = Core::get_instance().mm;
        /* フレーム作成 ********************************************************************/
        /* イメージ取得 */
        vk::Image color_buffers;
        vk::Image depth_buffers;
        vk::Image pick_buffers;

        // TODO widthとheightをextentに置き換え
        /* イメージ生成 */
        memory_manager.add_image("OffScreenColor0", extent.width, extent.height, NEGUI2::Image::TYPE::COLOR);
        memory_manager.add_image("OffScreenDepth0", extent.width, extent.height, NEGUI2::Image::TYPE::DEPTH);
        memory_manager.add_image("OffScreenPick0", extent.width, extent.height, NEGUI2::Image::TYPE::PICK);

        color_buffers = memory_manager.get_image("OffScreenColor0").image;
        depth_buffers = memory_manager.get_image("OffScreenDepth0").image;
        pick_buffers = memory_manager.get_image("OffScreenPick0").image;

        frame.color_buffer = color_buffers;
        frame.depth_buffer = depth_buffers;
        frame.pick_buffer = pick_buffers;

        color_format = memory_manager.get_image("OffScreenColor0").format;
        depth_format = memory_manager.get_image("OffScreenDepth0").format;
        pick_format = memory_manager.get_image("OffScreenPick0").format;

        /* Create RenderPass */
        {
            std::array<vk::AttachmentDescription, 3> attachmentDescriptions;
            attachmentDescriptions[0] = vk::AttachmentDescription({},
                                                                  color_format,
                                                                  vk::SampleCountFlagBits::e1,
                                                                  vk::AttachmentLoadOp::eClear,
                                                                  vk::AttachmentStoreOp::eStore,
                                                                  vk::AttachmentLoadOp::eDontCare,
                                                                  vk::AttachmentStoreOp::eDontCare,
                                                                  vk::ImageLayout::eUndefined,
                                                                  vk::ImageLayout::eGeneral);
            attachmentDescriptions[1] = vk::AttachmentDescription({},
                                                                  depth_format,
                                                                  vk::SampleCountFlagBits::e1,
                                                                  vk::AttachmentLoadOp::eClear,
                                                                  vk::AttachmentStoreOp::eDontCare,
                                                                  vk::AttachmentLoadOp::eDontCare,
                                                                  vk::AttachmentStoreOp::eDontCare,
                                                                  vk::ImageLayout::eUndefined,
                                                                  vk::ImageLayout::eDepthStencilAttachmentOptimal);
            attachmentDescriptions[2] = vk::AttachmentDescription({},
                                                                  pick_format,
                                                                  vk::SampleCountFlagBits::e1,
                                                                  vk::AttachmentLoadOp::eClear,
                                                                  vk::AttachmentStoreOp::eDontCare,
                                                                  vk::AttachmentLoadOp::eDontCare,
                                                                  vk::AttachmentStoreOp::eDontCare,
                                                                  vk::ImageLayout::eUndefined,
                                                                  vk::ImageLayout::eGeneral);

            vk::AttachmentReference colorReference(0, vk::ImageLayout::eGeneral);
            vk::AttachmentReference pickReference(2, vk::ImageLayout::eGeneral);
            std::array<vk::AttachmentReference, 2> references;
            references[0] = vk::AttachmentReference(0, vk::ImageLayout::eGeneral);
            references[1] = vk::AttachmentReference(2, vk::ImageLayout::eGeneral);
            vk::AttachmentReference depthReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
            vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, {}, references, {}, &depthReference);

            vk::RenderPassCreateInfo renderPassCreateInfo({}, attachmentDescriptions, subpass);
            render_pass = device_manager.device.createRenderPass(renderPassCreateInfo);
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
            auto &device = Core::get_instance().gpu.device;
            sampler = device.createSampler(create_info);
        }

        /* イメージビュー作成 */

        /* 色 */
        vk::ImageViewCreateInfo color_view_create_info({}, {}, vk::ImageViewType::e2D, color_format, {}, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        color_view_create_info.image = frame.color_buffer;
        vk::ImageSubresourceRange color_image_range{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
        color_view_create_info.subresourceRange = color_image_range;
        frame.color_buffer_view = device_manager.device.createImageView(color_view_create_info);

        /* 深度 */
        vk::ImageViewCreateInfo depth_view_create_info({}, {}, vk::ImageViewType::e2D, depth_format, {}, {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1});
        depth_view_create_info.image = frame.depth_buffer;
        vk::ImageSubresourceRange depth_image_range{vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1};
        depth_view_create_info.subresourceRange = depth_image_range;
        frame.depth_buffer_view = device_manager.device.createImageView(depth_view_create_info);

        /* ピック */
        vk::ImageViewCreateInfo pick_view_create_info({}, {}, vk::ImageViewType::e2D, pick_format, {}, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        pick_view_create_info.image = frame.pick_buffer;
        vk::ImageSubresourceRange pick_image_range{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
        pick_view_create_info.subresourceRange = pick_image_range;
        frame.pick_buffer_view = device_manager.device.createImageView(pick_view_create_info);

        /* フレームバッファ作成 */
        vk::FramebufferCreateInfo info;
        info.renderPass = *render_pass;
        std::array<vk::ImageView, 3> target_view{*frame.color_buffer_view, *frame.depth_buffer_view, *frame.pick_buffer_view};
        info.setAttachments(target_view);
        info.width = extent.width;
        info.height = extent.height;
        info.layers = 1;
        frame.frame_buffer = device_manager.device.createFramebuffer(info);
    }

    Eigen::Vector4i OffScreenManager::pick(const uint32_t &x, const uint32_t &y)
    {
        auto &memory_manager = Core::get_instance().mm;
        Eigen::Vector4i ret = memory_manager.read_pixel("OffScreenPick0", x, y);

        return ret;
    }
}