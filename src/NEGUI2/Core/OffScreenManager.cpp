#include "NEGUI2/Core/OffScreenManager.hpp"
#include "NEGUI2/Core/Core.hpp"
#include <spdlog/spdlog.h>
namespace
{
    vk::SurfaceFormatKHR select_surface_format(vk::raii::PhysicalDevice &physical_device, vk::raii::SurfaceKHR &surface, const std::vector<vk::Format> &request_formats, vk::ColorSpaceKHR &request_color_space)
    {
        // Per Spec Format and View Format are expected to be the same unless VK_IMAGE_CREATE_MUTABLE_BIT was set at image creation
        // Assuming that the default behavior is without setting this bit, there is no need for separate Swapchain image and image view format
        // Additionally several new color spaces were introduced with Vulkan Spec v1.0.40,
        // hence we must make sure that a format with the mostly available color space, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, is found and used.
        auto avail_format = physical_device.getSurfaceFormatsKHR(*surface);

        // First check if only one format, VK_FORMAT_UNDEFINED, is available, which would imply that any format is available
        if (avail_format.size() == 1)
        {
            if (avail_format[0].format == vk::Format::eUndefined)
            {
                vk::SurfaceFormatKHR ret;
                ret.format = request_formats[0];
                ret.colorSpace = request_color_space;
                return ret;
            }
            else
            {
                // No point in searching another format
                return avail_format[0];
            }
        }
        else
        {
            // Request several formats, the first found will be used
            for (int request_i = 0; request_i < request_formats.size(); request_i++)
                for (uint32_t avail_i = 0; avail_i < avail_format.size(); avail_i++)
                    if (avail_format[avail_i].format == request_formats[request_i] && avail_format[avail_i].colorSpace == request_color_space)
                        return avail_format[avail_i];

            // If none of the requested image formats could be found, use the first available
            return avail_format[0];
        }
    }

    vk::PresentModeKHR select_present_mode(const vk::raii::PhysicalDevice &physical_device, const vk::raii::SurfaceKHR &surface)
    {
        std::array<vk::PresentModeKHR, 3> request_modes{vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eImmediate, vk::PresentModeKHR::eFifo};

        // Request a certain mode and confirm that it is available. If not use VK_PRESENT_MODE_FIFO_KHR which is mandatory
        auto avail_modes = physical_device.getSurfacePresentModesKHR(*surface);

        for (int request_i = 0; request_i < request_modes.size(); request_i++)
        {
            for (uint32_t avail_i = 0; avail_i < avail_modes.size(); avail_i++)
            {
                if (request_modes[request_i] == avail_modes[avail_i])
                {
                    return request_modes[request_i];
                }
            }
        }

        return vk::PresentModeKHR::eFifo; // Always available
    }
}

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
            memory_manager.remove_image("OffScreenColor0");
            memory_manager.remove_image("OffScreenColor1");
            memory_manager.add_image("OffScreenColor0", width, height, NEGUI2::Image::TYPE::COLOR);
            memory_manager.add_image("OffScreenColor1", width, height, NEGUI2::Image::TYPE::COLOR);
            memory_manager.remove_image("OffScreenDepth0");
            memory_manager.remove_image("OffScreenDepth1");
            memory_manager.add_image("OffScreenDepth0", width, height, NEGUI2::Image::TYPE::DEPTH);
            memory_manager.add_image("OffScreenDepth1", width, height, NEGUI2::Image::TYPE::DEPTH);
            color_buffers[0] = *memory_manager.get_image("OffScreenColor0").image;
            color_buffers[1] = *memory_manager.get_image("OffScreenColor1").image;
            depth_buffers[0] = *memory_manager.get_image("OffScreenDepth0").image;
            depth_buffers[1] = *memory_manager.get_image("OffScreenDepth1").image;
            frames.resize(image_count);
            for (int i = 0; i < image_count; i++)
            {
                frames[i].color_buffer = color_buffers[i];
                frames[i].depth_buffer = depth_buffers[i];
            }
        }

        /* イメージビュー作成 */
        color_format = memory_manager.get_image("OffScreenColor0").format;
        depth_format = memory_manager.get_image("OffScreenDepth0").format;
        for (int i = 0; i < image_count; i++)
        {
            vk::ImageViewCreateInfo imageViewCreateInfo({}, {}, vk::ImageViewType::e2D, color_format, {}, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
            imageViewCreateInfo.image = frames[i].color_buffer;
            vk::ImageSubresourceRange image_range{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
            imageViewCreateInfo.subresourceRange = image_range;
            frames[i].back_buffer_view = device_manager.device.createImageView(imageViewCreateInfo);
        }

        /* フレームバッファ作成 */
        for (int i = 0; i < image_count; i++)
        {
            vk::FramebufferCreateInfo info;
            info.renderPass = *render_pass;
            std::array<vk::ImageView, 1> target_view{*frames[i].back_buffer_view};
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

        /* Create RenderPass */
        {
            std::array<vk::AttachmentDescription, 1> attachmentDescriptions;
            attachmentDescriptions[0] = vk::AttachmentDescription({},
                                                                  color_format,
                                                                  vk::SampleCountFlagBits::e1,
                                                                  vk::AttachmentLoadOp::eClear,
                                                                  vk::AttachmentStoreOp::eStore,
                                                                  vk::AttachmentLoadOp::eDontCare,
                                                                  vk::AttachmentStoreOp::eDontCare,
                                                                  vk::ImageLayout::eUndefined,
                                                                  vk::ImageLayout::ePresentSrcKHR);
#if 0
            attachmentDescriptions[1] = vk::AttachmentDescription({},
                                                                  depthFormat,
                                                                  vk::SampleCountFlagBits::e1,
                                                                  vk::AttachmentLoadOp::eClear,
                                                                  vk::AttachmentStoreOp::eDontCare,
                                                                  vk::AttachmentLoadOp::eDontCare,
                                                                  vk::AttachmentStoreOp::eDontCare,
                                                                  vk::ImageLayout::eUndefined,
                                                                  vk::ImageLayout::eDepthStencilAttachmentOptimal);
            vk::AttachmentReference depthReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
#endif // TODO Depthバッファ有効
            vk::AttachmentReference colorReference(0, vk::ImageLayout::eColorAttachmentOptimal);
            vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, {}, colorReference);

            vk::RenderPassCreateInfo renderPassCreateInfo({}, attachmentDescriptions, subpass);
            render_pass = device_manager.device.createRenderPass(renderPassCreateInfo);
        }
    }
}