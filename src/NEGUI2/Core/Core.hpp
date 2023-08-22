#ifndef _CORE_HPP
#define _CORE_HPP
#include <cstdint>
#include <functional>
#include <stack>
#include <memory>
#include <unordered_map>
#include <volk.h>
#include "NEGUI2/Core/Window.hpp"
#include "NEGUI2/Core/Shader.hpp"
#include "NEGUI2/3D/Camera.hpp"
#include <vk_mem_alloc.h>

namespace NEGUI2
{
    /* 前方宣言 */
    class IUserInterface;
    class I3DObject;
    
    /* 型宣言 */
    struct Memory
    {
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo alloc_info;

        enum class TYPE : uint32_t
        {
            VERTEX = 1,
            INDEX = 2,
            UNIFORM = 3
        };
        TYPE type;
    };

    struct MappedMemory
    {
        void *data;
        VmaAllocation allocation;
    };

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
        VmaVulkanFunctions vulkanFunctions;
        VmaAllocator allocator;
        Shader shader;
        VkCommandPool command_pool;
        VkCommandBuffer command_buffer;
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

    struct TextureData
    {
        VkCommandPool command_pool;
        VkCommandBuffer command_buffer;

        Memory color_memory;
        VkImage color_image;
        VkImageView back_color_view;
        Memory depth_memory;
        VkImage depth_image;
        VkImageView depth_view;
        VkFramebuffer frame_buffer;
        VkSampler sampler;
    };

    struct OffScreenData
    {
        int width;
        int height;
        VkRenderPass render_pass;
        uint32_t frame_index;     // Current frame being rendered to (0 <= FrameIndex < FrameInFlightCount)
        std::vector<TextureData> frames;
    };

    class Core
    {
        DeviceData device_data_;
        WindowData window_data_;
        std::stack<std::function<void(void)>> deletion_stack_;
        Window window_;
        Camera camera_;
        std::unordered_map<std::string, std::shared_ptr<IUserInterface>> user_interfaces_;
        std::unordered_map<std::string, std::shared_ptr<I3DObject>> objects_;
        std::unordered_map<std::string, Memory> memories_;

        void create_or_resize_window_();
        void setup_imgui_();
        Core();
    public:
        static Core& get_instance();
    
        void init();
        void update();
        bool should_colse() const;
        void destroy();
        bool add_userinterface(const std::string& key, std::shared_ptr<IUserInterface> ui);
        bool remove_userinteface(const std::string& key);
        bool add_3d_object(const std::string& key, std::shared_ptr<I3DObject> object);
        bool remove_3d_object(const std::string& key);
        
        Memory& get_memory(const std::string& key);
        bool add_memory(const std::string& key, const size_t& size, const Memory::TYPE& type);
        bool remove_memory(const std::string& key);
        bool upload_memory(const std::string& key, const void* data, const size_t size);
    };
}
#endif