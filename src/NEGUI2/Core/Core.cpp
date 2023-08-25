#include "NEGUI2/Core/Core.hpp"

namespace NEGUI2 {
    Core::Core() : initialized_(false), device_manager_(), memory_manager_(), screen_manager_(),
    offscreen_manager_()
    {
    }

    void Core::init()
    {
        initialized_ = true;
        window_.init();
        device_manager_.init();
        memory_manager_.init();
        screen_manager_.init();
        offscreen_manager_.init();
        offscreen_manager_.rebuild();
    }

    Core& Core::get_instance()
    {
        static Core core;
        if(!core.initialized_)
        {
            core.init();
        }

        return core;
    }

    DeviceManager& Core::get_device_manager()
    {
        return device_manager_;
    }

    MemoryManager& Core::get_memory_manager()
    {
        return memory_manager_;
    }

    Window& Core::get_window()
    {
        return window_;
    }

    ScreenManager& Core::get_screen_manager()
    {
        return screen_manager_;
    }

}