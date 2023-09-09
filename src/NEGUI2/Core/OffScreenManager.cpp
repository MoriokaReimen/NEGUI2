#include "NEGUI2/Core/OffScreenManager.hpp"
#include "NEGUI2/Core/Core.hpp"
#include <spdlog/spdlog.h>

namespace NEGUI2
{
    OffScreenManager::OffScreenManager() : width(0), height(0),
                                           render_pass(nullptr),
                                           clear_value(), swap_chain_rebuild(false),
                                           frame_index(0u), image_count(0u), semaphore_index(0u),
                                           frames(), sync_objects()

    {
    }

    void OffScreenManager::init()
    {
        auto &window = Core::get_instance().get_window();
        auto &device_manager = Core::get_instance().get_device_manager();

        {
            width = 100;
            height = 100;
        }

        /* 背景色設定 */
        {
            clear_value = {{166.0f / 256.0f, 205.0f / 256.0f, 182.0f / 256.0f, 0.0f}};
        }

        rebuild();
    }

    void OffScreenManager::rebuild()
    {
        /* カウンタリセット */
        frame_index = 0u; // Current frame being rendered to (0 <= FrameIndex < FrameInFlightCount)
        image_count = 2u; // Number of simultaneous in-flight frames (returned by vkGetSwapchainImagesKHR, usually derived from min_image_count)
        semaphore_index = 0u;

        auto &device_manager = Core::get_instance().get_device_manager();
        auto &memory_manager = Core::get_instance().get_memory_manager();

        /* フレーム作成 ********************************************************************/
        /* イメージ取得 */
        std::vector<vk::Image> color_buffers;
        std::vector<vk::Image> depth_buffers;
        {
            image_count = 2;
            color_buffers.resize(2);
            depth_buffers.resize(2);
            memory_manager.add_image("OffScreenColor0", width, height, NEGUI2::Image::TYPE::COLOR);
            memory_manager.add_image("OffScreenColor1", width, height, NEGUI2::Image::TYPE::COLOR);
            memory_manager.add_image("OffScreenDepth0", width, height, NEGUI2::Image::TYPE::DEPTH);
            memory_manager.add_image("OffScreenDepth1", width, height, NEGUI2::Image::TYPE::DEPTH);
            color_buffers[0] = memory_manager.get_image("OffScreenColor0").image;
            color_buffers[1] = memory_manager.get_image("OffScreenColor1").image;
            depth_buffers[0] = memory_manager.get_image("OffScreenDepth0").image;
            depth_buffers[1] = memory_manager.get_image("OffScreenDepth1").image;
            frames.resize(image_count);
            for (int i = 0; i < image_count; i++)
            {
                frames[i].color_buffer = color_buffers[i];
                frames[i].depth_buffer = depth_buffers[i];
            }
        }

        color_format = memory_manager.get_image("OffScreenColor0").format;
        depth_format = memory_manager.get_image("OffScreenDepth0").format;
        /* Create RenderPass */
        {
            std::array<vk::AttachmentDescription, 2> attachmentDescriptions;
            attachmentDescriptions[0] = vk::AttachmentDescription({},
                                                                  color_format,
                                                                  vk::SampleCountFlagBits::e1,
                                                                  vk::AttachmentLoadOp::eClear,
                                                                  vk::AttachmentStoreOp::eStore,
                                                                  vk::AttachmentLoadOp::eDontCare,
                                                                  vk::AttachmentStoreOp::eDontCare,
                                                                  vk::ImageLayout::eUndefined,
                                                                  vk::ImageLayout::ePresentSrcKHR);
            attachmentDescriptions[1] = vk::AttachmentDescription({},
                                                                  depth_format,
                                                                  vk::SampleCountFlagBits::e1,
                                                                  vk::AttachmentLoadOp::eClear,
                                                                  vk::AttachmentStoreOp::eDontCare,
                                                                  vk::AttachmentLoadOp::eDontCare,
                                                                  vk::AttachmentStoreOp::eDontCare,
                                                                  vk::ImageLayout::eUndefined,
                                                                  vk::ImageLayout::eDepthStencilAttachmentOptimal);

            vk::AttachmentReference colorReference(0, vk::ImageLayout::eColorAttachmentOptimal);
            vk::AttachmentReference depthReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
            vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, {}, colorReference, {}, &depthReference);

            vk::RenderPassCreateInfo renderPassCreateInfo({}, attachmentDescriptions, subpass);
            render_pass = device_manager.device.createRenderPass(renderPassCreateInfo);
        }

        /* イメージビュー作成 */
        for (int i = 0; i < image_count; i++)
        { //TODO メモリマネージャに移動
            /* 色 */
            vk::ImageViewCreateInfo color_view_create_info({}, {}, vk::ImageViewType::e2D, color_format, {}, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
            color_view_create_info.image = frames[i].color_buffer;
            vk::ImageSubresourceRange color_image_range{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
            color_view_create_info.subresourceRange = color_image_range;
            frames[i].color_buffer_view = device_manager.device.createImageView(color_view_create_info);

            /* 深度 */
            vk::ImageViewCreateInfo depth_view_create_info({}, {}, vk::ImageViewType::e2D, depth_format, {}, {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1});
            depth_view_create_info.image = frames[i].depth_buffer;
            vk::ImageSubresourceRange depth_image_range{vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1};
            depth_view_create_info.subresourceRange = depth_image_range;
            frames[i].depth_buffer_view = device_manager.device.createImageView(depth_view_create_info);
        }

        /* フレームバッファ作成 */
        for (int i = 0; i < image_count; i++)
        {
            vk::FramebufferCreateInfo info;
            info.renderPass = *render_pass;
            std::array<vk::ImageView, 2> target_view{*frames[i].color_buffer_view, *frames[i].depth_buffer_view};
            info.setAttachments(target_view);
            info.width = width;
            info.height = height;
            info.layers = 1;
            frames[i].frame_buffer = device_manager.device.createFramebuffer(info);
        }

        /* フェンス作成 */
        for (int i = 0; i < image_count; i++)
        {
            frames[i].fence = device_manager.device.createFence({vk::FenceCreateFlagBits::eSignaled});
        }

        /* セマフォ作成 */
        sync_objects.resize(image_count);
        for (int i = 0; i < image_count; i++)
        {
            sync_objects[i].image_acquired_semaphore = device_manager.device.createSemaphore({});
            sync_objects[i].image_rendered_semaphore = device_manager.device.createSemaphore({});
        }
    }
}