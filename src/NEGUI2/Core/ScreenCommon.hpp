#ifndef _SCREEN_COMMON_HPP
#define _SCREEN_COMMON_HPP
#include <vulkan/vulkan_raii.hpp>

namespace NEGUI2
{
    struct FrameData
    {
        vk::raii::Fence fence = nullptr;
        vk::Image color_buffer = nullptr;
        vk::Image depth_buffer = nullptr;
        vk::Image pick_buffer = nullptr;
        vk::raii::ImageView color_buffer_view = nullptr;
        vk::raii::ImageView depth_buffer_view = nullptr;
        vk::raii::ImageView pick_buffer_view = nullptr;
        vk::raii::Framebuffer frame_buffer = nullptr;
    };

    struct SyncObject
    {
        vk::raii::Semaphore image_acquired_semaphore = nullptr;
        vk::raii::Semaphore image_rendered_semaphore = nullptr;
    };
}

#endif