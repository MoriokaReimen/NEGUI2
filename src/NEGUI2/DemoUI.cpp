#include "NEGUI2/DemoUI.hpp"
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

namespace NEGUI2
{
    DemoUI::DemoUI()
        : IUserInterface::IUserInterface()
    {
    }

    DemoUI::~DemoUI()
    {
    }

    void DemoUI::update()
    {
        if (is_active_)
        {
            ImGui::ShowDemoWindow(&is_active_);
        }
    }
}