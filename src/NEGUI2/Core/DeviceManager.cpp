#include "NEGUI2/Core/DeviceManager.hpp"
#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>
#include <set>
#include <exception>
#include <iostream>
namespace
{
    bool is_renderable(const vk::raii::Instance &instance, const vk::raii::PhysicalDevice physical_device)
    {
        bool ret = false;
        auto queue_properties = physical_device.getQueueFamilyProperties();
        for (int i = 0; i < queue_properties.size(); i++)
        {
            if (glfwGetPhysicalDevicePresentationSupport(*instance, *physical_device, i))
            {
                ret = true;
                break;
            }
        }
        return ret;
    }

#ifndef NDEBUG
    VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char *pLayerPrefix, const char *pMessage, void *pUserData)
    {
        (void)flags;
        (void)object;
        (void)location;
        (void)messageCode;
        (void)pUserData;
        (void)pLayerPrefix; // Unused arguments
        if (flags & VkDebugReportFlagBitsEXT::VK_DEBUG_REPORT_ERROR_BIT_EXT)
        {
            spdlog::error("[vulkan] Debug report from ObjectType: {}\nMessage: {}", static_cast<int>(objectType), pMessage);
            throw std::runtime_error("Vulkan Error");
        }
        else
        {
            spdlog::info("[vulkan] Debug report from ObjectType: {}\nMessage: {}", static_cast<int>(objectType), pMessage);
        }
        return VK_FALSE;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                               VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                               VkDebugUtilsMessengerCallbackDataEXT const *pCallbackData,
                                                               void * /*pUserData*/)
    {
        std::stringstream ss;
        ss << vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity)) << ": "
           << vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageTypes)) << ":\n";
        ss << std::string("\t") << "messageIDName   = <" << pCallbackData->pMessageIdName << ">\n";
        ss << std::string("\t") << "messageIdNumber = " << pCallbackData->messageIdNumber << "\n";
        ss << std::string("\t") << "message         = <" << pCallbackData->pMessage << ">\n";
        if (0 < pCallbackData->queueLabelCount)
        {
            ss << std::string("\t") << "Queue Labels:\n";
            for (uint32_t i = 0; i < pCallbackData->queueLabelCount; i++)
            {
                ss << std::string("\t\t") << "labelName = <" << pCallbackData->pQueueLabels[i].pLabelName << ">\n";
            }
        }

        if (0 < pCallbackData->cmdBufLabelCount)
        {
            ss << std::string("\t") << "CommandBuffer Labels:\n";
            for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; i++)
            {
                ss << std::string("\t\t") << "labelName = <" << pCallbackData->pCmdBufLabels[i].pLabelName << ">\n";
            }
        }

        if (0 < pCallbackData->objectCount)
        {
            ss << std::string("\t") << "Objects:\n";
            for (uint32_t i = 0; i < pCallbackData->objectCount; i++)
            {
                ss << std::string("\t\t") << "Object " << i << "\n";
                ss << std::string("\t\t\t") << "objectType   = " << vk::to_string(static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType))
                   << "\n";
                ss << std::string("\t\t\t") << "objectHandle = " << pCallbackData->pObjects[i].objectHandle << "\n";
                if (pCallbackData->pObjects[i].pObjectName)
                {
                    ss << std::string("\t\t\t") << "objectName   = <" << pCallbackData->pObjects[i].pObjectName << ">\n";
                }
            }
        }
        if (messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            spdlog::error("{}", ss.str());
        }
        else
        {
            spdlog::info("{}", ss.str());
        }

        return VK_TRUE;
    }

#endif /* NDEBUG*/
    bool is_extension_available(const std::vector<vk::ExtensionProperties> &properties, const char *extension)
    {
        for (const auto &p : properties)
        {
            if (std::strcmp(p.extensionName, extension) == 0)
                return true;
        }
        return false;
    }

    bool is_layer_available(const std::vector<vk::LayerProperties> properties, const char *extension)
    {
        for (auto &property : properties)
        {
            if (std::strcmp(property.layerName, extension))
            {
                return true;
            }
        }

        return false;
    }
}

namespace NEGUI2
{
    void DeviceManager::init_instance_()
    {
        spdlog::info("Initialize Instance");
        std::vector<const char *> instance_extensions;
        uint32_t extensions_count = 0;
        const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
        for (uint32_t i = 0; i < extensions_count; i++)
        {
            instance_extensions.push_back(glfw_extensions[i]);
        }

        vk::InstanceCreateInfo create_info;

        // Enumerate available extensions
        auto properties = context_.enumerateInstanceExtensionProperties();
        for (auto &property : properties)
        {
            spdlog::info("Available Instance Extensions:");
            spdlog::info("\t{} : Ver.{}.{}.{}.{}", property.extensionName,
                         vk::apiVersionVariant(property.specVersion), vk::apiVersionMajor(property.specVersion),
                         vk::apiVersionMinor(property.specVersion), vk::apiVersionPatch(property.specVersion));
        }
        // TODO check_vk_result(err);

        // Enable required extensions
        if (is_extension_available(properties, VK_KHR_SURFACE_EXTENSION_NAME))
            instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        if (is_extension_available(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
            instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        if (is_extension_available(properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
        {
            instance_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            create_info.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
        }

        /* 検証レイヤ有効 */
#ifndef NDEBUG
        if (is_extension_available(properties, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
        {
            instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        else
        {
            spdlog::error("{} not found", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        auto layer_properties = vk::enumerateInstanceLayerProperties();
        if (is_layer_available(layer_properties, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME))
        {
            std::array<const char *, 1> layers{"VK_LAYER_KHRONOS_validation"};
            create_info.setPEnabledLayerNames(layers);
        }
        else
        {
            spdlog::error("{} not found", VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
        }

        // std::array<vk::ValidationFeatureEnableEXT, 1> enabled{vk::ValidationFeatureEnableEXT::eBestPractices};
        vk::ValidationFeaturesEXT features({}, {});
        create_info.pNext = &features;

#endif

        /* インスタンス生成 */
        create_info.setPEnabledExtensionNames(instance_extensions);
        vk::ApplicationInfo app_info;
        app_info.setApiVersion(vk::ApiVersion12)
            .setPApplicationName("NEGUI2")
            .setApplicationVersion(vk::makeApiVersion(0, 1, 0, 0))
            .setPEngineName("NEGUI2")
            .setEngineVersion(vk::makeApiVersion(0, 1, 0, 0));
        create_info.setPApplicationInfo(&app_info);
        instance = context_.createInstance(create_info);

        // Setup the debug report callback
#ifndef NDEBUG
        {
            vk::DebugUtilsMessengerCreateInfoEXT create_info{{},
                                                             vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
                                                             vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                                                 vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
                                                             &::debugUtilsMessengerCallback};
            debug_func = instance.createDebugUtilsMessengerEXT(create_info);
        }
#endif
    }

    void DeviceManager::init_physical_device_()
    {
        spdlog::info("Initialize Physical Device");
        auto gpus = instance.enumeratePhysicalDevices();
        int max_score = 0;
        int selected_index = -1;
        for (int i = 0; i < gpus.size(); i++)
        {
            auto &gpu = gpus[i];

            if (!is_renderable(instance, gpu))
                continue;
            auto property = gpu.getProperties();
            uint32_t score  = property.limits.maxFramebufferWidth * property.limits.maxFramebufferHeight;
            if (max_score < score)
            {
                max_score = score;
                selected_index = i;
            }
        }
        if (selected_index >= 0)
        {
            physical_device = gpus[selected_index];
        }
        else {
            spdlog::error("No good Device Found");
            std::abort();
        }
    }

    void DeviceManager::init_device_()
    {

        /* キュー選択 */
        {
            uint32_t count;
            auto queue_properties = physical_device.getQueueFamilyProperties();

            for (uint32_t i = 0; i < queue_properties.size(); i++)
            {
                if (queue_properties[i].queueFlags & vk::QueueFlagBits::eGraphics)
                {
                    graphics_queue_index = i;
                    break;
                }
            }
            assert(graphics_queue_index != (uint32_t)-1);

            // TODO 効率的なプレゼントキュー
            present_queue_index = graphics_queue_index;
        }

        // Create Logical Device (with 1 queue)
        {
            spdlog::info("Initialize Device");
            std::vector<const char *> device_extensions;
            device_extensions.push_back("VK_KHR_swapchain");

            /* Enumerate physical device extension */
            auto properties = physical_device.enumerateDeviceExtensionProperties();

            /* Queueのデータ設定 */
            std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
            std::set<uint32_t> uniqueQueueFamilies = {graphics_queue_index, present_queue_index};
            constexpr float QUEUEPRIORITY = 1.f;
            for (uint32_t queueFamily : uniqueQueueFamilies)
            {
                vk::DeviceQueueCreateInfo queueCreateInfo({}, queueFamily, 1, &QUEUEPRIORITY);
                queueCreateInfos.emplace_back(queueCreateInfo);
            }

            vk::DeviceCreateInfo create_info;
            create_info.setPEnabledExtensionNames(device_extensions);
            create_info.setQueueCreateInfos(queueCreateInfos);
            
            /* 物理デバイス機能の選択 */
            vk::PhysicalDeviceFeatures features;
            features.setLogicOp(vk::True);
            features.setDepthBounds(vk::True);
            features.setDepthClamp(vk::True);
            features.setIndependentBlend(vk::True);
            features.setFragmentStoresAndAtomics(vk::True);
            create_info.setPEnabledFeatures(&features);
            device = physical_device.createDevice(create_info);
        }
    }

    void DeviceManager::init_queue_()
    {
        graphics_queue = device.getQueue(graphics_queue_index, 0);
        present_queue = device.getQueue(present_queue_index, 0);
    }

    void DeviceManager::init_descriptor_pool_()
    {
        spdlog::info("Initialize Descriptor Pool");
        std::vector<vk::DescriptorPoolSize> pool_sizes =
            {
                {vk::DescriptorType::eUniformBuffer, 1000},
                {vk::DescriptorType::eStorageBuffer, 1000},
                {vk::DescriptorType::eSampler, 1000},
                {vk::DescriptorType::eCombinedImageSampler, 1000},
                {vk::DescriptorType::eSampledImage, 1000},
                {vk::DescriptorType::eStorageImage, 1000},
                {vk::DescriptorType::eUniformTexelBuffer, 1000},
                {vk::DescriptorType::eStorageTexelBuffer, 1000},
                {vk::DescriptorType::eUniformBufferDynamic, 1000},
                {vk::DescriptorType::eStorageBufferDynamic, 1000},
                {vk::DescriptorType::eInputAttachment, 1000}};

        vk::DescriptorPoolCreateInfo pool_info;
        pool_info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        pool_info.maxSets = 100;
        pool_info.setPoolSizes(pool_sizes);
        descriptor_pool = device.createDescriptorPool(pool_info);

        vk::DescriptorSetLayoutCreateInfo create_info;
        std::array<vk::DescriptorSetLayoutBinding, 3> bindings;
        bindings[0] = vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
        bindings[1] = vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
        bindings[2] = vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
        create_info.setBindings(bindings);
        descriptor_set_layout = device.createDescriptorSetLayout(create_info);

        vk::DescriptorSetAllocateInfo alloc_info;
        alloc_info.setDescriptorPool(*descriptor_pool)
                  .setSetLayouts(*descriptor_set_layout);
        auto descriptors = device.allocateDescriptorSets(alloc_info);
        descriptor_set = std::move(descriptors[0]);
    }

    void DeviceManager::init_command_pool_()
    {
        vk::CommandPoolCreateInfo create_info;
        create_info.queueFamilyIndex = graphics_queue_index;
        create_info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
        command_pool = device.createCommandPool(create_info);
    }

    void DeviceManager::init_pipeline_cache_()
    {
        pipeline_cache = device.createPipelineCache({});
    }

    DeviceManager::DeviceManager()
        : context_(), instance(nullptr), physical_device(nullptr),
          device(nullptr), graphics_queue_index((uint32_t)-1), present_queue_index((uint32_t)-1),
          graphics_queue(nullptr), present_queue(nullptr), debug_func(nullptr),
          descriptor_pool(nullptr), descriptor_set_layout(nullptr), descriptor_set(nullptr),
          command_pool(nullptr), pipeline_cache(nullptr)
    {
    }

    void DeviceManager::init()
    {
        init_instance_();
        init_physical_device_();
        init_device_();
        init_queue_();
        init_descriptor_pool_();
        init_command_pool_();
        init_pipeline_cache_();
    }

    DeviceManager::~DeviceManager()
    {
        device.waitIdle();
    }

    vk::Result DeviceManager::one_shot(std::function<vk::Result(vk::raii::CommandBuffer &command_buffer)> func)
    {
        /* コマンドバッファ生成 */
        vk::raii::CommandBuffer command_buffer(nullptr);
        {
            vk::CommandBufferAllocateInfo alloc_info{};
            alloc_info.commandPool = *command_pool;
            alloc_info.commandBufferCount = 1;
            alloc_info.level = vk::CommandBufferLevel::ePrimary;
            command_buffer = std::move(device.allocateCommandBuffers(alloc_info).front());
        }

        command_buffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
        auto ret = func(command_buffer);
        command_buffer.end();

        vk::SubmitInfo submitInfo;
        submitInfo.setCommandBuffers(*command_buffer);
        graphics_queue.submit(submitInfo);
        graphics_queue.waitIdle();

        return ret;
    }
}
