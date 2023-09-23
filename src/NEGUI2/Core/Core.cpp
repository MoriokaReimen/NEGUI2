#include "NEGUI2/Core/Core.hpp"
#include "NEGUI2/3D/BasePickable.hpp"
#include <spdlog/spdlog.h>
#include <imgui.h>

namespace NEGUI2 {
    Core::Core() : initialized_(false), gpu(), mm(), screen(),
    off_screen()
    {
    }

    Core::~Core()
    {
        shader.destroy();
    }

    void Core::init()
    {
        initialized_ = true;
        window.init();
        gpu.init();
        mm.init();
        screen.init();
        off_screen.init();
        tm.init();
        imgui.init();
        shader.init();
        aabb.init();
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

    bool Core::should_close()
    {
        return window.should_close();
    }
    void Core::update()
    {
        glfwPollEvents();

        static uint32_t command_index = 0;
        command_index = (command_index + 1) % 4;
        vk::raii::CommandBuffer& command_buffer = screen.command_buffers[command_index];

        if(screen.swap_chain_rebuild)
        {
            screen.rebuild();
            for(auto object : display_objects)
            {
                object->rebuild();
            }
        }

        auto& image_acqurired_semaphore = screen.sync_objects[screen.semaphore_index].image_acquired_semaphore;
        auto image_err = screen.swap_chain.acquireNextImage(UINT64_MAX, *image_acqurired_semaphore, nullptr);
        auto frame_index = image_err.second;
        if(image_err.first == vk::Result::eErrorOutOfDateKHR || image_err.first == vk::Result::eSuboptimalKHR)
        {
            screen.swap_chain_rebuild = true;
            return;
        }

        {
            auto wait_err = gpu.device.waitForFences({*screen.frames[frame_index].fence}, true, UINT64_MAX);
            if(wait_err != vk::Result::eSuccess)
            {
                spdlog::error("Fence wait error");
            }

            gpu.device.resetFences({*screen.frames[frame_index].fence});
        }

        {
            command_buffer.begin({{vk::CommandBufferUsageFlagBits::eOneTimeSubmit}});
        }

        {
            vk::RenderPassBeginInfo begin_info;
            begin_info.setRenderPass(*off_screen.render_pass)
            .setFramebuffer(*off_screen.frame.frame_buffer)
            .setRenderArea({{0, 0}, {off_screen.extent}})
            .setClearValueCount(2).setPClearValues(off_screen.clear_value.data());

            command_buffer.beginRenderPass(begin_info,
                                           vk::SubpassContents::eInline);
            
            for (auto display_object : display_objects)
            {
                display_object->update(command_buffer);

                auto pickable = std::dynamic_pointer_cast<BasePickable>(display_object);
                if(pickable && pickable->display_aabb())
                {
                    auto base_transform = std::dynamic_pointer_cast<BaseTransform>(display_object);
                    if(base_transform)
                    { // TODO 継承関係の整理
                        auto transform = base_transform->get_transform();
                        aabb.set_transform(transform);
                        aabb.set_box(pickable->box());
                        aabb.render(command_buffer);
                    }
                }
            }
            command_buffer.endRenderPass();
        }

        // TODO 型のエラーintをuint32_tに変換
        {
            vk::RenderPassBeginInfo begin_info;
            begin_info.setRenderPass(*screen.render_pass)
            .setFramebuffer(*screen.frames[frame_index].frame_buffer)
            .setRenderArea({{0, 0}, {screen.extent}})
            .setClearValueCount(1).setPClearValues(&screen.clear_value);

            command_buffer.beginRenderPass(begin_info,
                                           vk::SubpassContents::eInline);
            imgui.update(command_buffer);
            command_buffer.endRenderPass();
        }

        command_buffer.end();

        auto& image_rendered_semaphore = screen.sync_objects[screen.semaphore_index].image_rendered_semaphore;
        vk::SubmitInfo info;
        vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        info.setWaitSemaphores(*image_acqurired_semaphore).setWaitDstStageMask(flags)
            .setCommandBufferCount(1).setPCommandBuffers(&*command_buffer)
            .setSignalSemaphoreCount(1).setPSignalSemaphores(&*image_rendered_semaphore);
        gpu.graphics_queue.submit({info}, *screen.frames[frame_index].fence);

        vk::PresentInfoKHR present_info;
        present_info.setWaitSemaphoreCount(1).setPWaitSemaphores(&*image_rendered_semaphore)
                    .setSwapchainCount(1).setPSwapchains(&*screen.swap_chain).setPImageIndices(&frame_index);
        
        try {
             auto present_err = gpu.present_queue.presentKHR(present_info);
        } catch(const vk::OutOfDateKHRError& error)
        {
            screen.swap_chain_rebuild = true;
            return;
        }

        screen.semaphore_index = (screen.semaphore_index + 1) % screen.image_count;
    }

    void Core::wait_idle()
    {
        gpu.device.waitIdle();
    }
}