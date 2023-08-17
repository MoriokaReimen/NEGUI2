#ifndef _CORE_HPP
#define _CORE_HPP
#include <cstdint>
#include <functional>
#include <stack>
#include <vulkan/vulkan.h>

namespace NEGUI2
{
    struct DeviceData
    {
        VkInstance instance;
        VkPhysicalDevice physical_device;
        VkDevice device;
        uint32_t graphics_queue_index;
        uint32_t present_queue_index;
        VkQueue graphics_queue;
        VkQueue present_queue;
        VkDebugReportCallbackEXT debug_report;
        VkDescriptorPool descriptor_pool;
    };

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

    class Core
    {
        DeviceData device_data_;
        WindowData window_data_;
        std::stack<std::function<void(void)>> deletion_stack_;

    public:
        Core();
        void init();
        void update();
        bool should_colse() const;
        void destroy();
    };
}
#endif