#ifndef _DEVICE_MANAGER_HPP
#define _DEVICE_MANAGER_HPP
#include <stack>
#include <vulkan/vulkan_raii.hpp>
#include <functional>

namespace NEGUI2
{
    class DeviceManager
    {
        friend class Core;

        vk::raii::Context context_;
        DeviceManager();
        DeviceManager(const DeviceManager& other) = delete;
        DeviceManager& operator=(const DeviceManager& other) = delete;
        void init();
        void init_instance_();
        void init_physical_device_();
        void init_device_();
        void init_queue_();
        void init_descriptor_pool_();
        void init_command_pool_();
        void init_pipeline_cache_();
    public:
        ~DeviceManager();
        vk::raii::Instance instance;
        vk::raii::PhysicalDevice physical_device;
        vk::raii::Device device;
        uint32_t graphics_queue_index;
        uint32_t present_queue_index;
        vk::raii::Queue graphics_queue;
        vk::raii::Queue present_queue;
        vk::raii::DebugUtilsMessengerEXT debug_func;
        vk::raii::DescriptorPool descriptor_pool;
        vk::raii::DescriptorSetLayout descriptor_set_layout;
        vk::raii::DescriptorSet descriptor_set;
        vk::raii::CommandPool command_pool;
        vk::raii::PipelineCache pipeline_cache;
        vk::Result one_shot(std::function<vk::Result(vk::raii::CommandBuffer &command_buffer)> func);
    };
};
#endif