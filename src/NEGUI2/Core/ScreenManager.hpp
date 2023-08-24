#ifndef _SCREEN_MANAGER_HPP
#define _SCREEN_MANAGER_HPP
#include <vulkan/vulkan_raii.hpp>
namespace NEGUI2
{

    struct FrameData
    {
        vk::raii::Fence fence = nullptr;
        vk::raii::Image back_buffer = nullptr;
        vk::raii::ImageView back_buffer_view = nullptr;
        vk::raii::Framebuffer frame_buffer = nullptr;
    };

    struct SyncObject
    {
        vk::raii::Semaphore image_acquired_semaphore  = nullptr;
        vk::raii::Semaphore image_rendered_semaphore  = nullptr;
    };
    class ScreenManager
    {
        friend class Core;
        ScreenManager();
        void init(); // TODO すべてのモジュールにデストロイを追加
        ScreenManager(const ScreenManager& other) = delete;
        ScreenManager& operator=(const ScreenManager& other) = delete;
    public:
        int width;
        int height;
        vk::raii::SwapchainKHR swap_chain;
        vk::raii::SurfaceKHR surface;
        vk::SurfaceFormatKHR surface_format;
        vk::PresentModeKHR present_mode;
        vk::raii::RenderPass render_pass;
        // vk::raii::Pipeline pipeline; // The window pipeline may uses a different VkRenderPass than the one passed in ImGui_ImplVulkan_InitInfo
        vk::ClearValue clear_value;
        bool swap_chain_rebuild;
        uint32_t frame_index;     // Current frame being rendered to (0 <= FrameIndex < FrameInFlightCount)
        uint32_t image_count;     // Number of simultaneous in-flight frames (returned by vkGetSwapchainImagesKHR, usually derived from min_image_count)
        uint32_t semaphore_index; // Current set of swapchain wait semaphores we're using (needs to be distinct from per frame data)
        std::vector<FrameData> frames;
        std::vector<SyncObject> sync_objects;

        void rebuild();
    };
}

#endif