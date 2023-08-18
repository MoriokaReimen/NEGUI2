#include "NEGUI2/UiDemo.hpp"
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

namespace NEGUI2
{
    UiDemo::UiDemo()
        : IUserInterface::IUserInterface()
    {
    }

    UiDemo::~UiDemo()
    {
    }

    void UiDemo::update()
    {
        if (is_active_)
        {
            ImGui::ShowDemoWindow(&is_active_);
        }
    }
}