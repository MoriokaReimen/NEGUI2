#include "NEGUI2/Core.hpp"
#include "NEGUI2/Utility.hpp"
#include <cassert>
#include <spdlog/spdlog.h>
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
namespace
{
#ifndef NDEBUG
    VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char *pLayerPrefix, const char *pMessage, void *pUserData)
    {
        (void)flags;
        (void)object;
        (void)location;
        (void)messageCode;
        (void)pUserData;
        (void)pLayerPrefix; // Unused arguments
        spdlog::error("[vulkan] Debug report from ObjectType: {}\nMessage: {}\n\n", static_cast<int>(objectType), pMessage);
        return VK_FALSE;
    }
#endif // IMGUI_VULKAN_DEBUG_REPORT

    bool is_extension_available(const std::vector<VkExtensionProperties> &properties, const char *extension)
    {
        for (const VkExtensionProperties &p : properties)
        {
            if (std::strcmp(p.extensionName, extension) == 0)
                return true;
        }
        return false;
    }

}

namespace NEGUI2
{
    Core::Core()
    {
    }

    void Core::init()
    {
        /* Initialize instance */
        {
            spdlog::info("Initialize Instance");
            std::vector<const char *> instance_extensions;
            uint32_t extensions_count = 0;
            const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
            for (uint32_t i = 0; i < extensions_count; i++)
            {
                instance_extensions.push_back(glfw_extensions[i]);
            }

            VkInstanceCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

            // Enumerate available extensions
            uint32_t properties_count;
            std::vector<VkExtensionProperties> properties;
            vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
            properties.resize(properties_count);
            VkResult err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, properties.data());
            check_vk_result(err);

            // Enable required extensions
            if (is_extension_available(properties, VK_KHR_SURFACE_EXTENSION_NAME))
                instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
            if (is_extension_available(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
                instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            if (is_extension_available(properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
            {
                instance_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
                create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
            }

            // Enabling validation layers
#ifndef NDEBUG
            const char *layers[] = {"VK_LAYER_KHRONOS_validation"};
            create_info.enabledLayerCount = 1;
            create_info.ppEnabledLayerNames = layers;
            instance_extensions.push_back("VK_EXT_debug_report");
#endif

            // Create Vulkan Instance
            create_info.enabledExtensionCount = (uint32_t)instance_extensions.size();
            create_info.ppEnabledExtensionNames = instance_extensions.data();
            VkResult create_err = vkCreateInstance(&create_info, nullptr, &device_data_.instance);
            check_vk_result(create_err);
            deletion_stack_.push([&]()
                                 {
                spdlog::info("Destroy Instance");
                vkDestroyInstance(device_data_.instance, nullptr); });

            // Setup the debug report callback
#ifndef NDEBUG
            auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(device_data_.instance, "vkCreateDebugReportCallbackEXT");
            assert(vkCreateDebugReportCallbackEXT != nullptr);
            VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
            debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
            debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            debug_report_ci.pfnCallback = ::debug_report;
            debug_report_ci.pUserData = nullptr;
            VkResult callback_err = vkCreateDebugReportCallbackEXT(device_data_.instance, &debug_report_ci, nullptr, &device_data_.debug_report);
            check_vk_result(callback_err);
            deletion_stack_.push([&]()
                                 {
                spdlog::info("Destroy Debug Report Callback");
                    auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(device_data_.instance, "vkDestroyDebugReportCallbackEXT");
                vkDestroyDebugReportCallbackEXT(device_data_.instance, device_data_.debug_report, nullptr); });
#endif
        }

        /* Physical Deviceの選定 */
        {
            spdlog::info("Initialize Physical Device");
            uint32_t gpu_count;
            VkResult err = vkEnumeratePhysicalDevices(device_data_.instance, &gpu_count, nullptr);
            NEGUI2::check_vk_result(err);
            assert(gpu_count > 0);

            std::vector<VkPhysicalDevice> gpus;
            gpus.resize(gpu_count);
            err = vkEnumeratePhysicalDevices(device_data_.instance, &gpu_count, gpus.data());
            NEGUI2::check_vk_result(err);

            // If a number >1 of GPUs got reported, find discrete GPU if present, or use first one available. This covers
            // most common cases (multi-gpu/integrated+dedicated graphics). Handling more complicated setups (multiple
            // dedicated GPUs) is out of scope of device_data_ sample.
            bool physical_device_found = false;
            for (VkPhysicalDevice &device : gpus)
            {
                VkPhysicalDeviceProperties properties;
                vkGetPhysicalDeviceProperties(device, &properties);
                if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                {
                    device_data_.physical_device = device;
                    physical_device_found = true;
                    break;
                }
            }

            // Use first GPU (Integrated) is a Discrete one is not available.
            if (gpu_count > 0 && !physical_device_found)
            {
                device_data_.physical_device = gpus[0];
            }
            else if (!physical_device_found)
            {
                assert(!"No Physical Device Found!");
            }
        }

        /* キュー選択 */
        {
            uint32_t count;
            vkGetPhysicalDeviceQueueFamilyProperties(device_data_.physical_device, &count, nullptr);
            std::vector<VkQueueFamilyProperties> queues;
            queues.resize(count);
            vkGetPhysicalDeviceQueueFamilyProperties(device_data_.physical_device, &count, queues.data());
            device_data_.graphics_queue_index = (uint32_t)-1;
            for (uint32_t i = 0; i < count; i++)
            {
                if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    device_data_.graphics_queue_index = i;
                    break;
                }
            }
            assert(device_data_.graphics_queue_index != (uint32_t)-1);

            // TODO 効率的なプレゼントキュー
            device_data_.present_queue_index = device_data_.graphics_queue_index;
        }

        // Create Logical Device (with 1 queue)
        {
            spdlog::info("Initialize Device");
            std::vector<const char *> device_extensions;
            device_extensions.push_back("VK_KHR_swapchain");

            // Enumerate physical device extension
            uint32_t properties_count;
            std::vector<VkExtensionProperties> properties;
            vkEnumerateDeviceExtensionProperties(device_data_.physical_device, nullptr, &properties_count, nullptr);
            properties.resize(properties_count);
            vkEnumerateDeviceExtensionProperties(device_data_.physical_device, nullptr, &properties_count, properties.data());

            const float queue_priority[] = {1.0f};
            VkDeviceQueueCreateInfo queue_info[1] = {};
            queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info[0].queueFamilyIndex = device_data_.graphics_queue_index;
            queue_info[0].queueCount = 1;
            queue_info[0].pQueuePriorities = queue_priority;
            VkDeviceCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
            create_info.pQueueCreateInfos = queue_info;
            create_info.enabledExtensionCount = (uint32_t)device_extensions.size();
            create_info.ppEnabledExtensionNames = device_extensions.data();
            VkResult create_err = vkCreateDevice(device_data_.physical_device, &create_info, nullptr, &device_data_.device);
            check_vk_result(create_err);
            vkGetDeviceQueue(device_data_.device, device_data_.graphics_queue_index, 0, &device_data_.graphics_queue);
            deletion_stack_.push([&]()
                                 {
                spdlog::info("Destroy Device");
                vkDestroyDevice(device_data_.device, nullptr); });
        }
        // If you wish to load e.g. additional textures you may need to alter pools sizes.
        {
            spdlog::info("Initialize Descriptor Pool");
            std::vector<VkDescriptorPoolSize> pool_sizes =
                {
                    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000}};
            VkDescriptorPoolCreateInfo pool_info = {};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            pool_info.maxSets = 1;
            pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
            pool_info.pPoolSizes = pool_sizes.data();
            VkResult err = vkCreateDescriptorPool(device_data_.device, &pool_info, nullptr, &device_data_.descriptor_pool);
            check_vk_result(err);
            deletion_stack_.push([&]()
                                 {
                spdlog::info("Destroy Descriptor Pool");
                vkDestroyDescriptorPool(device_data_.device, device_data_.descriptor_pool, nullptr); });
        }
    }

    void Core::update()
    {
    }

    bool Core::should_colse() const
    {

        return false;
    }

    void Core::destroy()
    {
        while (!deletion_stack_.empty())
        {
            deletion_stack_.top()();
            deletion_stack_.pop();
        }
    }
}