#include "NEGUI2/Ui/TextureDemo.hpp"
#include <fstream>
#include "NEGUI2/Core/Core.hpp"
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

namespace NEGUI2
{

    TextureDemo::TextureDemo()
        : IUserInterface::IUserInterface()
    {
        auto& core = Core::get_instance();
        
        auto& tm = core.get_texture_manager();
        tm.load_from_file("./runtime/idle.png");
        auto texture = tm.get("./runtime/idle.png");
        
        texture_id_ = ImGui_ImplVulkan_AddTexture(texture.sampler, texture.image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    TextureDemo::~TextureDemo()
    {
    }

    void TextureDemo::update()
    {
        ImGui::Begin("Texture Demo");
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        ImGui::Image(texture_id_, ImVec2{viewportPanelSize.x, viewportPanelSize.y});

        ImGui::End();
    }

}
