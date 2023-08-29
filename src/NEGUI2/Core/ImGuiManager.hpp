#ifndef _IMGUI_MANAGER_HPP
#define _IMGUI_MANAGER_HPP
#include <vulkan/vulkan_raii.hpp>
namespace NEGUI2
{
    class ImGuiManager
    {
        friend class Core;
        ImGuiManager();
        ImGuiManager(const ImGuiManager &other) = delete;
        ImGuiManager &operator=(const ImGuiManager &other) = delete;

    public:
        ~ImGuiManager();

        void init();
        void update(vk::raii::CommandBuffer& command_buffer);
    };

}

#endif