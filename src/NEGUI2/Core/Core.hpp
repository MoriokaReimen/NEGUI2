#ifndef _CORE_HPP
#define _CORE_HPP
#include "NEGUI2/Core/DeviceManager.hpp"
#include "NEGUI2/Core/MemoryManager.hpp"
#include "NEGUI2/Core/Window.hpp"
namespace NEGUI2
{
    class Core
    {
        bool initialized_;
        DeviceManager device_manager_;
        MemoryManager memory_manager_;
        Window window_;
// TODO DebugUtilityを導入
        Core();
        void init();
        Core(const Core& other) = delete;
        Core& operator=(const Core& other) = delete;
    public:
        static Core &get_instance();
        DeviceManager& get_device_manager();
        MemoryManager& get_memory_manager();
        Window& get_window();
    };
}
#endif