#ifndef _WINDOW_DATA_HPP
#define _WINDOW_DATA_HPP
#include <vector>
#include <vulkan/vulkan.h>

namespace NEGUI2
{
    struct FrameData
    {
        VkCommandPool CommandPool;
        VkCommandBuffer CommandBuffer;
        VkFence Fence;
        VkImage Backbuffer;
        VkImageView BackbufferView;
        VkFramebuffer Framebuffer;
    };

    struct SyncObject
    {
        VkSemaphore ImageAcquiredSemaphore;
        VkSemaphore RenderCompleteSemaphore;
    };

    struct WindowData
    {
        int Width;
        int Height;
        VkSwapchainKHR Swapchain;
        VkSurfaceKHR Surface;
        VkSurfaceFormatKHR SurfaceFormat;
        VkPresentModeKHR PresentMode;
        VkRenderPass RenderPass;
        VkPipeline Pipeline; // The window pipeline may uses a different VkRenderPass than the one passed in ImGui_ImplVulkan_InitInfo
        bool UseDynamicRendering;
        bool ClearEnable;
        VkClearValue ClearValue;
        uint32_t FrameIndex;     // Current frame being rendered to (0 <= FrameIndex < FrameInFlightCount)
        uint32_t ImageCount;     // Number of simultaneous in-flight frames (returned by vkGetSwapchainImagesKHR, usually derived from min_image_count)
        uint32_t SemaphoreIndex; // Current set of swapchain wait semaphores we're using (needs to be distinct from per frame data)
        std::vector<FrameData> frames;
        std::vector<SyncObject> sync_objects;
    };

}
#endif