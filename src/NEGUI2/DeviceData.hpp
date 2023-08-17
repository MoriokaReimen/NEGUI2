#ifndef _DEVICE_DATA_HPP
#define _DEVICE_DATA_HPP
#include <vulkan/vulkan.h>
#include <stack>
#include <functional>

namespace NEGUI2
{
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

        void init();
        void destroy();

    private:
        std::stack<std::function<void(void)>> delete_stack_;
    };
}
#endif