#ifndef _CORE_HPP
#define _CORE_HPP
#include "NEGUI2/Core/DeviceManager.hpp"
#include "NEGUI2/Core/MemoryManager.hpp"
#include "NEGUI2/Core/Window.hpp"
#include "NEGUI2/Core/ScreenManager.hpp"
#include "NEGUI2/Core/OffScreenManager.hpp"
#include "NEGUI2/Core/TextureManager.hpp"
#include "NEGUI2/Core/ImGuiManager.hpp"
#include "NEGUI2/Core/Shader.hpp"
#include "NEGUI2/3D/BaseDisplayObject.hpp"
#include "NEGUI2/3D/AABB.hpp"
#include <memory>

namespace NEGUI2
{
    class Core
    {
        bool initialized_;

        Core();
        void init();
        Core(const Core& other) = delete;
        Core& operator=(const Core& other) = delete;
    public:
        ~Core();
        static Core &get_instance();

        DeviceManager gpu;
        MemoryManager mm;
        TextureManager tm;
        Window window;
        ScreenManager screen;
        OffScreenManager off_screen;
        ImGuiManager imgui;
        AABB aabb;
        std::vector<std::shared_ptr<BaseDisplayObject>> display_objects;
        Shader shader;

        bool should_close();
        void update();
        void wait_idle();

    };
}
#endif