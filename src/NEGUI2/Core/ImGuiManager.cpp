#include "NEGUI2/Core/ImGuiManager.hpp"
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <implot.h>
#include <spdlog/spdlog.h>
#include "NEGUI2/Core/Core.hpp"

namespace
{
    void check_vk_result(VkResult err)
    {
        if (err == 0)
            return;
        spdlog::error("[vulkan] Error: VkResult = %d\n", err);
        if (err < 0)
            std::abort();
    }
}

namespace NEGUI2
{

    ImGuiManager::ImGuiManager()
    {
    }

    ImGuiManager::~ImGuiManager()
    {
        // TODO add destroy class
        // spdlog::info("Destroy ImGuiManager");
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
    }

    void ImGuiManager::init()
    {
        auto &core = Core::get_instance();
        auto &dm = core.get_device_manager();
        auto &sm = core.get_screen_manager();
        auto &window = core.get_window();
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui_ImplVulkan_LoadFunctions([](const char *function_name, void *vulkan_instance)
                                       { return reinterpret_cast<vk::raii::Instance *>(vulkan_instance)->getProcAddr(function_name); },
                                       &dm.instance);

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

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = *dm.instance;
        init_info.PhysicalDevice = *dm.physical_device;
        init_info.Device = *dm.device;
        init_info.QueueFamily = dm.graphics_queue_index;
        init_info.Queue = *dm.graphics_queue;
        init_info.PipelineCache = *dm.pipeline_cache;
        init_info.DescriptorPool = *dm.descriptor_pool;
        init_info.Subpass = 0;
        init_info.MinImageCount = 2;
        init_info.ImageCount = sm.image_count;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = NULL;
        init_info.CheckVkResultFn = check_vk_result;
        ImGui_ImplVulkan_Init(&init_info, *sm.render_pass);
        ImGui_ImplGlfw_InitForVulkan(window.get_window(), true);

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
        dm.one_shot([&](vk::raii::CommandBuffer &command_buffer)
                    { ImGui_ImplVulkan_CreateFontsTexture(*command_buffer);
                        return vk::Result::eSuccess; });
        ImGui_ImplVulkan_DestroyFontUploadObjects();

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiManager::update(vk::raii::CommandBuffer &command_buffer)
    {
        ImGui::Render();
        ImDrawData *draw_data = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(draw_data, *command_buffer);

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
}
