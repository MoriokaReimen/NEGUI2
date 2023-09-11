#ifndef _OFF_SCREEN_MANAGER_HPP
#define _OFF_SCREEN_MANAGER_HPP
#include <vulkan/vulkan_raii.hpp>
#include "NEGUI2/Core/ScreenCommon.hpp"

namespace NEGUI2
{
    class OffScreenManager
    {
        friend class Core;
        OffScreenManager();
        void init(); // TODO すべてのモジュールにデストロイを追加
        OffScreenManager(const OffScreenManager& other) = delete;
        OffScreenManager& operator=(const OffScreenManager& other) = delete;
    public:
        vk::Extent2D extent;
        vk::raii::RenderPass render_pass;
        vk::ClearValue clear_value;
        vk::Format color_format;
        vk::Format depth_format;
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