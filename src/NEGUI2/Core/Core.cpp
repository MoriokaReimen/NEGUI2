#include "NEGUI2/Core/Core.hpp"
#include <spdlog/spdlog.h>
#include <imgui.h>

namespace NEGUI2 {
    Core::Core() : initialized_(false), device_manager_(), memory_manager_(), screen_manager_(),
    offscreen_manager_()
    {
    }

    Core::~Core()
    {
        shader_.destroy();
    }

    void Core::init()
    {
        initialized_ = true;
        window_.init();
        device_manager_.init();
        memory_manager_.init();
        screen_manager_.init();
        offscreen_manager_.init();
        offscreen_manager_.rebuild();
        texture_manager_.init();
        imgui_manager_.init();
        shader_.init();
    }

    Core& Core::get_instance()
    {
        static Core core;
        if(!core.initialized_)
        {
            core.init();
        }

        return core;
    }

    DeviceManager& Core::get_device_manager()
    {
        return device_manager_;
    }

    MemoryManager& Core::get_memory_manager()
    {
        return memory_manager_;
    }

    Window& Core::get_window()
    {
        return window_;
    }

    ScreenManager& Core::get_screen_manager()
    {
        return screen_manager_;
    }

    TextureManager& Core::get_texture_manager()
    {
        return texture_manager_;
    }

    ImGuiManager& Core::get_imgui_manager()
    {
        return imgui_manager_;
    }

    Shader& Core::get_shader()
    {
        return shader_;
    }

    bool Core::should_close()
    {
        return window_.should_close();
    }
    void Core::update()
    {
        glfwPollEvents();

        static uint32_t command_index = 0;
        command_index = (command_index + 1) % 4;
        vk::raii::CommandBuffer& command_buffer = screen_manager_.command_buffers[command_index];

        if(screen_manager_.swap_chain_rebuild)
        {
            screen_manager_.rebuild();
            for(auto object : display_objects)
            {
                object->rebuild();
            }
        }

        auto& image_acqurired_semaphore = screen_manager_.sync_objects[screen_manager_.semaphore_index].image_acquired_semaphore;
        auto image_err = screen_manager_.swap_chain.acquireNextImage(UINT64_MAX, *image_acqurired_semaphore, nullptr);
        auto frame_index = image_err.second;
        if(image_err.first == vk::Result::eErrorOutOfDateKHR || image_err.first == vk::Result::eSuboptimalKHR)
        {
            screen_manager_.swap_chain_rebuild = true;
            return;
        }

        {
            auto wait_err = device_manager_.device.waitForFences({*screen_manager_.frames[frame_index].fence}, true, UINT64_MAX);
            if(wait_err != vk::Result::eSuccess)
            {
                spdlog::error("Fence wait error");
            }

            device_manager_.device.resetFences({*screen_manager_.frames[frame_index].fence});
        }

        {
            command_buffer.begin({{vk::CommandBufferUsageFlagBits::eOneTimeSubmit}});
        }

        // TODO 型のエラーintをuint32_tに変換
        {
            vk::RenderPassBeginInfo begin_info;
            begin_info.setRenderPass(*screen_manager_.render_pass)
            .setFramebuffer(*screen_manager_.frames[frame_index].frame_buffer)
            .setRenderArea({{0, 0}, {screen_manager_.extent}})
            .setClearValueCount(1).setPClearValues(&screen_manager_.clear_value);

            command_buffer.beginRenderPass(begin_info,
                                           vk::SubpassContents::eInline);
        }

        imgui_manager_.update(command_buffer);
        for(auto display_object : display_objects)
        {
            display_object->update(command_buffer);
        }

        command_buffer.endRenderPass();
        command_buffer.end();

        auto& image_rendered_semaphore = screen_manager_.sync_objects[screen_manager_.semaphore_index].image_rendered_semaphore;
        vk::SubmitInfo info;
        vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        info.setWaitSemaphores(*image_acqurired_semaphore).setWaitDstStageMask(flags)
            .setCommandBufferCount(1).setPCommandBuffers(&*command_buffer)
            .setSignalSemaphoreCount(1).setPSignalSemaphores(&*image_rendered_semaphore);
        device_manager_.graphics_queue.submit({info}, *screen_manager_.frames[frame_index].fence);

        vk::PresentInfoKHR present_info;
        present_info.setWaitSemaphoreCount(1).setPWaitSemaphores(&*image_rendered_semaphore)
                    .setSwapchainCount(1).setPSwapchains(&*screen_manager_.swap_chain).setPImageIndices(&frame_index);
        
        try {
             auto present_err = device_manager_.present_queue.presentKHR(present_info);
        } catch(const vk::OutOfDateKHRError& error)
        {
            screen_manager_.swap_chain_rebuild = true;
            return;
        }

        screen_manager_.semaphore_index = (screen_manager_.semaphore_index + 1) % screen_manager_.image_count;
    }

    void Core::wait_idle()
    {
        device_manager_.device.waitIdle();
    }
}