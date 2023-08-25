#include "NEGUI2/Core/ScreenManager.hpp"
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
    ScreenManager::ScreenManager() : width(0), height(0),
                                     swap_chain(nullptr), surface(nullptr),
                                     surface_format(), present_mode(), render_pass(nullptr),
                                     clear_value(), swap_chain_rebuild(false),
                                     frame_index(0u), image_count(0u), semaphore_index(0u),
                                     frames(), sync_objects(0u)

    {
    }

    void ScreenManager::init()
    {
        auto &window = Core::get_instance().get_window();
        auto &device_manager = Core::get_instance().get_device_manager();

        {
            window.get_extent(width, height);
        }

        {
            VkSurfaceKHR temp_surface;
            glfwCreateWindowSurface(*device_manager.instance, window.get_window(), nullptr, &temp_surface);
            surface = vk::raii::SurfaceKHR(device_manager.instance, temp_surface);
        }

        /* Check for WSI support */
        {
            auto ret = device_manager.physical_device.getSurfaceSupportKHR(device_manager.graphics_queue_index, *surface);
            if (!ret)
                spdlog::error("WSI not supported");
        }

        /* Select Surface Format */
        {
            const std::vector<vk::Format> requestSurfaceImageFormat = {vk::Format::eB8G8R8A8Unorm, vk::Format::eR8G8B8A8Unorm, vk::Format::eB8G8R8Unorm, vk::Format::eR8G8B8Unorm};
            auto requestSurfaceColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
            surface_format = ::select_surface_format(device_manager.physical_device, surface, requestSurfaceImageFormat, requestSurfaceColorSpace);
        }

        {
            present_mode = ::select_present_mode(device_manager.physical_device, surface);
        }

        /* 背景色設定 */
        {
            clear_value = {{166.0f / 256.0f, 205.0f / 256.0f, 182.0f / 256.0f, 0.0f}};
        }

        rebuild();
#if 0
        vk::raii::SwapchainKHR swap_chain;
        vk::raii::RenderPass render_pass;
        uint32_t frame_index;     // Current frame being rendered to (0 <= FrameIndex < FrameInFlightCount)
        uint32_t image_count;     // Number of simultaneous in-flight frames (returned by vkGetSwapchainImagesKHR, usually derived from min_image_count)
        uint32_t semaphore_index; // Current set of swapchain wait semaphores we're using (needs to be distinct from per frame data)
        std::vector<FrameData> frames;
        std::vector<SyncObject> sync_objects;
#endif
    }

    void ScreenManager::rebuild()
    {
        swap_chain_rebuild = false;
        frame_index = 0u;     // Current frame being rendered to (0 <= FrameIndex < FrameInFlightCount)
        image_count = 0u;     // Number of simultaneous in-flight frames (returned by vkGetSwapchainImagesKHR, usually derived from min_image_count)
        semaphore_index = 0u;
        vk::raii::SwapchainKHR old_swapchain = std::move(swap_chain);
        auto &device_manager = Core::get_instance().get_device_manager();
        
        /* Create Swap Chain */
        {
            vk::SwapchainCreateInfoKHR info = {};
            info.surface = *surface;
            info.minImageCount = 2;
            info.imageFormat = surface_format.format;
            info.imageColorSpace = surface_format.colorSpace;
            info.imageArrayLayers = 1;
            info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
            info.imageSharingMode = vk::SharingMode::eExclusive; // Assume that graphics family == present family
            info.preTransform =  vk::SurfaceTransformFlagBitsKHR::eIdentity;
            info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
            info.presentMode = present_mode;
            info.clipped = vk::True;
            info.oldSwapchain = *old_swapchain;

            vk::SurfaceCapabilitiesKHR cap = device_manager.physical_device.getSurfaceCapabilitiesKHR(*surface);
            if (info.minImageCount < cap.minImageCount)
                info.minImageCount = cap.minImageCount;
            else if (cap.maxImageCount != 0 && info.minImageCount > cap.maxImageCount)
                info.minImageCount = cap.maxImageCount;

            if (cap.currentExtent.width == 0xffffffff)
            {        
                auto &window = Core::get_instance().get_window();
                window.get_extent(width, height);
                info.imageExtent.width = width;
                info.imageExtent.height = height;
            }
            else
            {
                info.imageExtent.width = width = cap.currentExtent.width;
                info.imageExtent.height = height = cap.currentExtent.height;
            }
            swap_chain = device_manager.device.createSwapchainKHR(info);
        }

        /* Create Images */
        std::vector<vk::Image> back_buffers;
        {
            back_buffers = swap_chain.getImages();
            image_count = back_buffers.size();
            frames.resize(image_count);
            for(int i = 0; i < image_count; i++)
            {
                frames[i].color_buffer = back_buffers[i];
            }
        }

        /* Create RenderPass */
        {
            vk::Format colorFormat = surface_format.format;
            vk::Format depthFormat = vk::Format::eD16Unorm;

            std::array<vk::AttachmentDescription, 1> attachmentDescriptions;
            attachmentDescriptions[0] = vk::AttachmentDescription({},
                                                                  colorFormat,
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

        frames.resize(image_count);
        for(int i = 0; i < image_count; i++)
        {
            frames[i].fence = device_manager.device.createFence({vk::FenceCreateFlagBits::eSignaled});
            vk::ImageViewCreateInfo imageViewCreateInfo( {}, {}, vk::ImageViewType::e2D, surface_format.format, {}, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } );
            imageViewCreateInfo.image = frames[i].color_buffer;
            vk::ImageSubresourceRange image_range{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
            imageViewCreateInfo.subresourceRange = image_range;    
            frames[i].back_buffer_view = device_manager.device.createImageView(imageViewCreateInfo);
// TODO ルールを知る
            vk::FramebufferCreateInfo info;
            info.renderPass = *render_pass;
            std::array<vk::ImageView, 1> target_view{*frames[i].back_buffer_view};
            info.setAttachments(target_view);
            info.width = width;
            info.height = height;
            info.layers = 1;
            frames[i].frame_buffer = device_manager.device.createFramebuffer(info);

        }

        sync_objects.resize(image_count);
        for(int i = 0; i < image_count; i++)
        {
            sync_objects[i].image_acquired_semaphore = device_manager.device.createSemaphore({});
            sync_objects[i].image_rendered_semaphore = device_manager.device.createSemaphore({});
        }
    }
}