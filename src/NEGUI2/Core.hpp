#ifndef _CORE_HPP
#define _CORE_HPP
#include <cstdint>
#include <functional>
#include <stack>
#include <vulkan/vulkan.h>
#include "NEGUI2/WindowData.hpp"
#include "NEGUI2/DeviceData.hpp"

namespace NEGUI2
{
    class Core
    {
        DeviceData device_data_;
        WindowData window_data_;
        std::stack<std::function<void(void)>> deletion_stack_;

    public:
        Core();
        void init();
        void update();
        bool should_colse() const;
    };
}
#endif