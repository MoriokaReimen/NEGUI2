#ifndef _CORE_HPP
#define _CORE_HPP
#include <cstdint>
#include <functional>
#include <stack>
#include <vulkan/vulkan.h>
#include "NEGUI2/Window.hpp"
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
        VkCommandPool command_pool;
        VkCommandBuffer command_buffer;
        VkFence fence;
        VkImage back_buffer;
        VkImageView back_buffer_view;
        VkFramebuffer frame_buffer;
    };

    struct SyncObject
    {
        VkSemaphore image_acquired_semaphore;
        VkSemaphore image_acquire_semaphore;
    };

    struct WindowData
    {
        int width;
        int height;
        VkSwapchainKHR swap_chain;
        VkSurfaceKHR surface;
        VkSurfaceFormatKHR surface_format;
        VkPresentModeKHR present_mode;
        VkRenderPass render_pass;
        VkPipeline pipeline; // The window pipeline may uses a different VkRenderPass than the one passed in ImGui_ImplVulkan_InitInfo
        bool use_dynamic_rendering;
        bool clear_enable;
        VkClearValue clear_value;
        bool swap_chain_rebuild;
        uint32_t frame_index;     // Current frame being rendered to (0 <= FrameIndex < FrameInFlightCount)
        uint32_t image_count;     // Number of simultaneous in-flight frames (returned by vkGetSwapchainImagesKHR, usually derived from min_image_count)
        uint32_t semaphore_index; // Current set of swapchain wait semaphores we're using (needs to be distinct from per frame data)
        std::vector<FrameData> frames;
        std::vector<SyncObject> sync_objects;
    };

    class Core
    {
        DeviceData device_data_;
        WindowData window_data_;
        std::stack<std::function<void(void)>> deletion_stack_;
        Window window_;

        void create_or_resize_window_();
        void setup_imgui_();

    public:
        Core();
        void init();
        void update();
        bool should_colse() const;
        void destroy();
    };
}
#endif