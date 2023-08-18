#include "NEGUI2/Core.hpp"
#include "NEGUI2/Utility.hpp"
#include <set>
#include <cassert>
#include "NEGUI2/IUserInterface.hpp"
#include <spdlog/spdlog.h>
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <implot.h>
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

    VkSurfaceFormatKHR select_surface_format(VkPhysicalDevice physical_device, VkSurfaceKHR surface, const std::vector<VkFormat> &request_formats, VkColorSpaceKHR request_color_space)
    {
        // Per Spec Format and View Format are expected to be the same unless VK_IMAGE_CREATE_MUTABLE_BIT was set at image creation
        // Assuming that the default behavior is without setting this bit, there is no need for separate Swapchain image and image view format
        // Additionally several new color spaces were introduced with Vulkan Spec v1.0.40,
        // hence we must make sure that a format with the mostly available color space, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, is found and used.
        uint32_t avail_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &avail_count, nullptr);
        std::vector<VkSurfaceFormatKHR> avail_format;
        avail_format.resize((int)avail_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &avail_count, avail_format.data());

        // First check if only one format, VK_FORMAT_UNDEFINED, is available, which would imply that any format is available
        if (avail_count == 1)
        {
            if (avail_format[0].format == VK_FORMAT_UNDEFINED)
            {
                VkSurfaceFormatKHR ret;
                ret.format = request_formats[0];
                ret.colorSpace = request_color_space;
                return ret;
            }
            else
            {
                // No point in searching another format
                return avail_format[0];
            }
        }
        else
        {
            // Request several formats, the first found will be used
            for (int request_i = 0; request_i < request_formats.size(); request_i++)
                for (uint32_t avail_i = 0; avail_i < avail_count; avail_i++)
                    if (avail_format[avail_i].format == request_formats[request_i] && avail_format[avail_i].colorSpace == request_color_space)
                        return avail_format[avail_i];

            // If none of the requested image formats could be found, use the first available
            return avail_format[0];
        }
    }

    VkPresentModeKHR select_present_mode(const VkPhysicalDevice &physical_device, const VkSurfaceKHR &surface)
    {
        std::array<VkPresentModeKHR, 3> request_modes{VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR};

        // Request a certain mode and confirm that it is available. If not use VK_PRESENT_MODE_FIFO_KHR which is mandatory
        uint32_t avail_count = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &avail_count, nullptr);
        std::vector<VkPresentModeKHR> avail_modes;
        avail_modes.resize((int)avail_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &avail_count, avail_modes.data());

        for (int request_i = 0; request_i < request_modes.size(); request_i++)
        {
            for (uint32_t avail_i = 0; avail_i < avail_count; avail_i++)
            {
                if (request_modes[request_i] == avail_modes[avail_i])
                {
                    return request_modes[request_i];
                }
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR; // Always available
    }
}

namespace NEGUI2
{
    void Core::create_or_resize_window_()
    {
        VkResult err;
        VkSwapchainKHR old_swapchain = window_data_.swap_chain;
        window_data_.swap_chain = VK_NULL_HANDLE;

        /* 待機 */
        {
            err = vkDeviceWaitIdle(device_data_.device);
            check_vk_result(err);
        }

        // We don't use ImGui_ImplVulkanH_DestroyWindow() because we want to preserve the old swapchain to create the new one.
        // Destroy old Framebuffer
        for (uint32_t i = 0; i < window_data_.image_count; i++)
        {
            vkDestroyCommandPool(device_data_.device, window_data_.frames[i].command_pool, nullptr);
            vkDestroyFence(device_data_.device, window_data_.frames[i].fence, nullptr);
            vkDestroyFramebuffer(device_data_.device, window_data_.frames[i].frame_buffer, nullptr);
            vkDestroyImageView(device_data_.device, window_data_.frames[i].back_buffer_view, nullptr);
            vkDestroySemaphore(device_data_.device, window_data_.sync_objects[i].image_acquired_semaphore, nullptr);
            vkDestroySemaphore(device_data_.device, window_data_.sync_objects[i].image_acquire_semaphore, nullptr);
        }
        window_data_.image_count = 0;

        if (window_data_.render_pass)
            vkDestroyRenderPass(device_data_.device, window_data_.render_pass, nullptr);
        if (window_data_.pipeline)
            vkDestroyPipeline(device_data_.device, window_data_.pipeline, nullptr);

        // Create Swapchain
        {
            VkSwapchainCreateInfoKHR info = {};
            info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            info.surface = window_data_.surface;
            info.minImageCount = 2;
            info.imageFormat = window_data_.surface_format.format;
            info.imageColorSpace = window_data_.surface_format.colorSpace;
            info.imageArrayLayers = 1;
            info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // Assume that graphics family == present family
            info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
            info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            info.presentMode = window_data_.present_mode;
            info.clipped = VK_TRUE;
            info.oldSwapchain = old_swapchain;
            VkSurfaceCapabilitiesKHR cap;
            err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_data_.physical_device, window_data_.surface, &cap);
            check_vk_result(err);
            if (info.minImageCount < cap.minImageCount)
                info.minImageCount = cap.minImageCount;
            else if (cap.maxImageCount != 0 && info.minImageCount > cap.maxImageCount)
                info.minImageCount = cap.maxImageCount;

            if (cap.currentExtent.width == 0xffffffff)
            {
                int width, height;
                window_.get_extent(width, height);
                info.imageExtent.width = window_data_.width = width;
                info.imageExtent.height = window_data_.height = height;
            }
            else
            {
                info.imageExtent.width = window_data_.width = cap.currentExtent.width;
                info.imageExtent.height = window_data_.height = cap.currentExtent.height;
            }
            err = vkCreateSwapchainKHR(device_data_.device, &info, nullptr, &window_data_.swap_chain);
            check_vk_result(err);
            err = vkGetSwapchainImagesKHR(device_data_.device, window_data_.swap_chain, &window_data_.image_count, nullptr);
            check_vk_result(err);
            std::vector<VkImage> backbuffers;
            backbuffers.resize(window_data_.image_count);
            err = vkGetSwapchainImagesKHR(device_data_.device, window_data_.swap_chain, &window_data_.image_count, backbuffers.data());
            check_vk_result(err);

            window_data_.frames.resize(window_data_.image_count);
            window_data_.sync_objects.resize(window_data_.image_count);

            std::memset(window_data_.frames.data(), 0, sizeof(window_data_.frames[0]) * window_data_.image_count);
            std::memset(window_data_.sync_objects.data(), 0, sizeof(window_data_.sync_objects[0]) * window_data_.image_count);
            for (uint32_t i = 0; i < window_data_.image_count; i++)
                window_data_.frames[i].back_buffer = backbuffers[i];
        }
        if (old_swapchain)
            vkDestroySwapchainKHR(device_data_.device, old_swapchain, nullptr);

        // Create the Render Pass
        if (window_data_.use_dynamic_rendering == false)
        {
            VkAttachmentDescription attachment = {};
            attachment.format = window_data_.surface_format.format;
            attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            attachment.loadOp = window_data_.clear_enable ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            VkAttachmentReference color_attachment = {};
            color_attachment.attachment = 0;
            color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            VkSubpassDescription subpass = {};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &color_attachment;
            VkSubpassDependency dependency = {};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.srcAccessMask = 0;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            VkRenderPassCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            info.attachmentCount = 1;
            info.pAttachments = &attachment;
            info.subpassCount = 1;
            info.pSubpasses = &subpass;
            info.dependencyCount = 1;
            info.pDependencies = &dependency;
            err = vkCreateRenderPass(device_data_.device, &info, nullptr, &window_data_.render_pass);
            check_vk_result(err);
        }

        // Create The Image Views
        {
            VkImageViewCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            info.format = window_data_.surface_format.format;
            info.components.r = VK_COMPONENT_SWIZZLE_R;
            info.components.g = VK_COMPONENT_SWIZZLE_G;
            info.components.b = VK_COMPONENT_SWIZZLE_B;
            info.components.a = VK_COMPONENT_SWIZZLE_A;
            VkImageSubresourceRange image_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
            info.subresourceRange = image_range;
            for (uint32_t i = 0; i < window_data_.image_count; i++)
            {
                info.image = window_data_.frames[i].back_buffer;
                err = vkCreateImageView(device_data_.device, &info, nullptr, &window_data_.frames[i].back_buffer_view);
                check_vk_result(err);
            }
        }

        // Create Framebuffer
        if (window_data_.use_dynamic_rendering == false)
        {
            VkImageView attachment[1];
            VkFramebufferCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            info.renderPass = window_data_.render_pass;
            info.attachmentCount = 1;
            info.pAttachments = attachment;
            info.width = window_data_.width;
            info.height = window_data_.height;
            info.layers = 1;
            for (uint32_t i = 0; i < window_data_.image_count; i++)
            {
                attachment[0] = window_data_.frames[i].back_buffer_view;
                err = vkCreateFramebuffer(device_data_.device, &info, nullptr, &window_data_.frames[i].frame_buffer);
                check_vk_result(err);
            }

            window_data_.swap_chain_rebuild = false;
            window_data_.frame_index = 0;
        }

        /* コマンドプール生成 */
        for (uint32_t i = 0; i < window_data_.image_count; i++)
        {
            VkCommandPoolCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            create_info.queueFamilyIndex = device_data_.graphics_queue_index;
            create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            create_info.pNext = nullptr;

            auto err = vkCreateCommandPool(device_data_.device, &create_info, nullptr, &window_data_.frames[i].command_pool);
            check_vk_result(err);
        }

        /*コマンドバッファ生成 */
        for (uint32_t i = 0; i < window_data_.image_count; i++)
        {
            VkCommandBufferAllocateInfo alloc_info{};
            alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            alloc_info.commandPool = window_data_.frames[i].command_pool;
            alloc_info.commandBufferCount = 1;
            alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            alloc_info.pNext = nullptr;
            auto err = vkAllocateCommandBuffers(device_data_.device, &alloc_info, &window_data_.frames[i].command_buffer);
            check_vk_result(err);
        }

        /* フェンス生成 */
        for (uint32_t i = 0; i < window_data_.image_count; i++)
        {
            VkFenceCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            create_info.pNext = nullptr;
            auto err = vkCreateFence(device_data_.device, &create_info, nullptr, &window_data_.frames[i].fence);
            check_vk_result(err);
        }

        /* セマフォ生成 */
        for (uint32_t i = 0; i < window_data_.image_count; i++)
        {
            VkSemaphoreCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            create_info.pNext = nullptr;
            auto err = vkCreateSemaphore(device_data_.device, &create_info, nullptr, &window_data_.sync_objects[i].image_acquired_semaphore);
            check_vk_result(err);
            err = vkCreateSemaphore(device_data_.device, &create_info, nullptr, &window_data_.sync_objects[i].image_acquire_semaphore);
            check_vk_result(err);
        }
    }

    void Core::setup_imgui_()
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForVulkan(window_.get_window(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = device_data_.instance;
        init_info.PhysicalDevice = device_data_.physical_device;
        init_info.Device = device_data_.device;
        init_info.QueueFamily = device_data_.graphics_queue_index;
        init_info.Queue = device_data_.graphics_queue;
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = device_data_.descriptor_pool;
        init_info.Subpass = 0;
        init_info.MinImageCount = 2;
        init_info.ImageCount = window_data_.image_count;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = nullptr;
        init_info.CheckVkResultFn = check_vk_result;
        ImGui_ImplVulkan_Init(&init_info, window_data_.render_pass);

        // Load Fonts
        // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
        // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
        // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
        // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
        // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
        // - Read 'docs/FONTS.md' for more instructions and details.
        // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
        // io.Fonts->AddFontDefault();
        // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
        // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
        // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
        // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
        // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
        // IM_ASSERT(font != nullptr);

        // Upload Fonts
        {
            // Use any command queue
            VkCommandPool command_pool = window_data_.frames[window_data_.frame_index].command_pool;
            VkCommandBuffer command_buffer = window_data_.frames[window_data_.frame_index].command_buffer;

            auto reset_err = vkResetCommandPool(device_data_.device, command_pool, 0);
            check_vk_result(reset_err);
            VkCommandBufferBeginInfo begin_info = {};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            auto begin_err = vkBeginCommandBuffer(command_buffer, &begin_info);
            check_vk_result(begin_err);

            ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

            VkSubmitInfo end_info = {};
            end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            end_info.commandBufferCount = 1;
            end_info.pCommandBuffers = &command_buffer;
            auto end_err = vkEndCommandBuffer(command_buffer);
            check_vk_result(end_err);
            auto submit_err = vkQueueSubmit(device_data_.graphics_queue, 1, &end_info, VK_NULL_HANDLE);
            check_vk_result(submit_err);

            auto wait_err = vkDeviceWaitIdle(device_data_.device);
            check_vk_result(wait_err);
            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }
    }

    Core::Core() : window_data_{0}
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

            /* Queueのデータ設定 */
            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            std::set<uint32_t> uniqueQueueFamilies = {device_data_.graphics_queue_index, device_data_.present_queue_index};
            constexpr float QUEUEPRIORITY = 1.0f;
            for (uint32_t queueFamily : uniqueQueueFamilies)
            {
                VkDeviceQueueCreateInfo queueCreateInfo{};
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = queueFamily;
                queueCreateInfo.queueCount = 1;
                queueCreateInfo.pQueuePriorities = &QUEUEPRIORITY;
                queueCreateInfos.emplace_back(queueCreateInfo);
            }

            VkDeviceCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            create_info.queueCreateInfoCount = queueCreateInfos.size();
            create_info.pQueueCreateInfos = queueCreateInfos.data();
            create_info.enabledExtensionCount = (uint32_t)device_extensions.size();
            create_info.ppEnabledExtensionNames = device_extensions.data();
            VkResult create_err = vkCreateDevice(device_data_.physical_device, &create_info, nullptr, &device_data_.device);
            check_vk_result(create_err);
            deletion_stack_.push([&]()
                                 {
                spdlog::info("Destroy Device");
                vkDestroyDevice(device_data_.device, nullptr); });
        }

        /* キュー生成 */
        {
            vkGetDeviceQueue(device_data_.device, device_data_.graphics_queue_index, 0, &device_data_.graphics_queue);
            vkGetDeviceQueue(device_data_.device, device_data_.present_queue_index, 0, &device_data_.present_queue);
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

        // Create Window Surface
        {
            auto window = window_.get_window();
            VkResult err = glfwCreateWindowSurface(device_data_.instance, window, nullptr, &window_data_.surface);
            check_vk_result(err);
            deletion_stack_.push([&]()
                                 {
                spdlog::info("Destroy Surface");
                vkDestroySurfaceKHR(device_data_.instance, window_data_.surface, nullptr); });
        }

        /* Window Size */
        {
            window_.get_extent(window_data_.width, window_data_.height);
        }

        /* Check for WSI support */
        {
            VkBool32 res;
            vkGetPhysicalDeviceSurfaceSupportKHR(device_data_.physical_device, device_data_.graphics_queue_index, window_data_.surface, &res);
            if (res != VK_TRUE)
            {
                spdlog::error("WSI not supported");
                exit(-1);
            }
        }

        /* Select Surface Format */
        { // TODO Optimize Surface Format
            const std::vector<VkFormat> requestSurfaceImageFormat = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
            const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
            window_data_.surface_format = ::select_surface_format(device_data_.physical_device, window_data_.surface, requestSurfaceImageFormat, requestSurfaceColorSpace);
        }

        /* Select Present Mode */
        {
            window_data_.present_mode = ::select_present_mode(device_data_.physical_device, window_data_.surface);
        }

        /* 背景色設定 */
        {
            window_data_.clear_enable = true;
            VkClearValue clear_value{};
            VkClearColorValue color = {166.0f / 256.0f, 205.0f / 256.0f, 182.0f / 256.0f, 0.0f};
            clear_value.color = color;
            window_data_.clear_value = clear_value;
        }

        /* フレームデータ生成 */
        create_or_resize_window_();

        /* imgui初期化 */
        setup_imgui_();
    }

    void Core::update()
    {
        glfwPollEvents();

        /* SwapChainを再構築 */
        if (window_data_.swap_chain_rebuild)
        {
            int width, height;
            window_.get_extent(width, height);
            if (width > 0 && height > 0)
            {
                create_or_resize_window_();
            }
        }

        /* イメージ取得 */
        {
            VkSemaphore image_acquired_semaphore = window_data_.sync_objects[window_data_.semaphore_index].image_acquired_semaphore;
            auto image_err = vkAcquireNextImageKHR(device_data_.device, window_data_.swap_chain, UINT64_MAX, image_acquired_semaphore, nullptr, &window_data_.frame_index);
            if (image_err == VK_ERROR_OUT_OF_DATE_KHR || image_err == VK_SUBOPTIMAL_KHR)
            {
                window_data_.swap_chain_rebuild = true;
                return;
            }
            check_vk_result(image_err);
        }

        /* 完了まで待機 */
        {
            auto wait_err = vkWaitForFences(device_data_.device, 1, &window_data_.frames[window_data_.frame_index].fence, VK_TRUE, UINT64_MAX); // wait indefinitely instead of periodically checking
            check_vk_result(wait_err);

            auto reset_err = vkResetFences(device_data_.device, 1, &window_data_.frames[window_data_.frame_index].fence);
            check_vk_result(reset_err);
        }

        /* コマンド開始 */
        {
            VkCommandBufferBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            auto begin_err = vkBeginCommandBuffer(window_data_.frames[window_data_.frame_index].command_buffer, &info);
            check_vk_result(begin_err);
        }

        /* レンダーパス開始 */
        {
            VkRenderPassBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            info.renderPass = window_data_.render_pass;
            info.framebuffer = window_data_.frames[window_data_.frame_index].frame_buffer;
            info.renderArea.extent.width = window_data_.width;
            info.renderArea.extent.height = window_data_.height;
            info.clearValueCount = 1;
            info.pClearValues = &window_data_.clear_value;
            vkCmdBeginRenderPass(window_data_.frames[window_data_.frame_index].command_buffer, &info, VK_SUBPASS_CONTENTS_INLINE);
        }

        /* UIを描画 */
        {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            for (auto &ui : user_interfaces_)
            {
                ui.second->update();
            }

            ImGui::Render();
            ImDrawData *draw_data = ImGui::GetDrawData();
            ImGui_ImplVulkan_RenderDrawData(draw_data, window_data_.frames[window_data_.frame_index].command_buffer);
        }

        /* フレーム終了 */
        {
            vkCmdEndRenderPass(window_data_.frames[window_data_.frame_index].command_buffer);
            auto err = vkEndCommandBuffer(window_data_.frames[window_data_.frame_index].command_buffer);
            check_vk_result(err);
        }

        /* キューにアップロード */
        {
            VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            VkSemaphore image_acquired_semaphore = window_data_.sync_objects[window_data_.semaphore_index].image_acquired_semaphore;
            VkSemaphore render_complete_semaphore = window_data_.sync_objects[window_data_.semaphore_index].image_acquired_semaphore;

            VkSubmitInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            info.waitSemaphoreCount = 1;
            info.pWaitSemaphores = &image_acquired_semaphore;
            info.pWaitDstStageMask = &wait_stage;
            info.commandBufferCount = 1;
            info.pCommandBuffers = &window_data_.frames[window_data_.frame_index].command_buffer;
            info.signalSemaphoreCount = 1;
            info.pSignalSemaphores = &render_complete_semaphore;
            auto submit_err = vkQueueSubmit(device_data_.graphics_queue, 1, &info, window_data_.frames[window_data_.frame_index].fence);
            check_vk_result(submit_err);
        }

        /* プレゼント指示 */
        {
            VkSemaphore render_complete_semaphore = window_data_.sync_objects[window_data_.semaphore_index].image_acquired_semaphore;

            VkPresentInfoKHR info = {};
            info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            info.waitSemaphoreCount = 1;
            info.pWaitSemaphores = &render_complete_semaphore;
            info.swapchainCount = 1;
            info.pSwapchains = &window_data_.swap_chain;
            info.pImageIndices = &window_data_.frame_index;
            VkResult err = vkQueuePresentKHR(device_data_.present_queue, &info);
            if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
            {
                window_data_.swap_chain_rebuild = true;
                return;
            }
            check_vk_result(err);
            window_data_.semaphore_index = (window_data_.semaphore_index + 1) % window_data_.image_count; // Now we can use the next set of semaphores
        }
    }

    bool Core::should_colse() const
    {
        return window_.should_close();
    }

    void Core::destroy()
    {
        /* 終了まで待機 */
        {
            auto err = vkDeviceWaitIdle(device_data_.device);
            check_vk_result(err);
        }

        /* imguiを解放 */
        {
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImPlot::DestroyContext();
            ImGui::DestroyContext();
        }

        // We don't use ImGui_ImplVulkanH_DestroyWindow() because we want to preserve the old swapchain to create the new one.
        // Destroy old Framebuffer
        for (uint32_t i = 0; i < window_data_.image_count; i++)
        {
            vkDestroyCommandPool(device_data_.device, window_data_.frames[i].command_pool, nullptr);
            vkDestroyFence(device_data_.device, window_data_.frames[i].fence, nullptr);
            vkDestroyFramebuffer(device_data_.device, window_data_.frames[i].frame_buffer, nullptr);
            vkDestroyImageView(device_data_.device, window_data_.frames[i].back_buffer_view, nullptr);
            vkDestroySemaphore(device_data_.device, window_data_.sync_objects[i].image_acquired_semaphore, nullptr);
            vkDestroySemaphore(device_data_.device, window_data_.sync_objects[i].image_acquire_semaphore, nullptr);
        }
        window_data_.image_count = 0;

        if (window_data_.render_pass)
            vkDestroyRenderPass(device_data_.device, window_data_.render_pass, nullptr);
        if (window_data_.pipeline)
            vkDestroyPipeline(device_data_.device, window_data_.pipeline, nullptr);

        vkDestroySwapchainKHR(device_data_.device, window_data_.swap_chain, nullptr);

        while (!deletion_stack_.empty())
        {
            deletion_stack_.top()();
            deletion_stack_.pop();
        }
    }

    bool Core::add_userinterface(const std::string &key, std::shared_ptr<IUserInterface> ui)
    {
        bool ret = false;
        if (user_interfaces_.count(key) == 0)
        {
            user_interfaces_.insert({key, ui});
            ret = true;
        }

        return ret;
    }

    bool Core::remove_userinteface(const std::string &key)
    {
        bool ret = false;
        if (user_interfaces_.count(key) != 0)
        {
            user_interfaces_.erase(key);
            ret = true;
        }

        return ret;
    }
}
