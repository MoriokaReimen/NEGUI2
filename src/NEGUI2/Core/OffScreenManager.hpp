#ifndef _OFF_SCREEN_MANAGER_HPP
#define _OFF_SCREEN_MANAGER_HPP
#include <vulkan/vulkan_raii.hpp>
#include "NEGUI2/Core/ScreenCommon.hpp"
#include <Eigen/Dense>

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
        vk::raii::Sampler sampler;
        std::array<vk::ClearValue, 3> clear_value;
        vk::Format color_format;
        vk::Format depth_format;
        vk::Format pick_format;
        bool swap_chain_rebuild;
        FrameData frame;
        void rebuild();
        Eigen::Vector4i pick(const uint32_t& x, const uint32_t& y);
    };
}
#endif