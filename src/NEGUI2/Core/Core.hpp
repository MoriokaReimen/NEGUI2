#ifndef _CORE_HPP
#define _CORE_HPP
#include "NEGUI2/Core/DeviceManager.hpp"
#include "NEGUI2/Core/MemoryManager.hpp"
#include "NEGUI2/Core/Window.hpp"
#include "NEGUI2/Core/ScreenManager.hpp"
#include "NEGUI2/Core/OffScreenManager.hpp"
#include "NEGUI2/Core/TextureManager.hpp"
#include "NEGUI2/Core/ImGuiManager.hpp"
namespace NEGUI2
{
    class Core
    {
        bool initialized_;
        DeviceManager device_manager_;
        MemoryManager memory_manager_;
        Window window_;
        ScreenManager screen_manager_;
        OffScreenManager offscreen_manager_;
        TextureManager texture_manager_;
        ImGuiManager imgui_manager_;

        Core();
        void init();
        Core(const Core& other) = delete;
        Core& operator=(const Core& other) = delete;
    public:
        static Core &get_instance();
        DeviceManager& get_device_manager();
        MemoryManager& get_memory_manager();
        Window& get_window();
        ScreenManager& get_screen_manager();
        TextureManager& get_texture_manager();
        ImGuiManager& get_imgui_manager();

        bool should_close();
        void update();
        void wait_idle();
    };
}
#endif