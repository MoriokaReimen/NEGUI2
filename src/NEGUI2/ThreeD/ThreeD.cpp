#include "NEGUI2/ThreeD/ThreeD.hpp"
#include "NEGUI2/ThreeD/BasePickable.hpp"
#include <limits>
#include "NEGUI2/Core/Core.hpp"
#include <spdlog/spdlog.h>

namespace NEGUI2
{

    ThreeD::ThreeD()
        : display_objects_(), camera_()
    {
    }

    ThreeD::~ThreeD()
    {
    }

    void ThreeD::init()
    {
        camera_.init();
        aabb_.init();

        auto &core = Core::get_instance();
        auto &mm = core.mm;
        {
            mm.add_memory("pick_data", sizeof(PickData), Memory::TYPE::SSBO);
        }

        auto &gpu = core.gpu;
        {
            std::array<vk::DescriptorBufferInfo, 1> buffer_infos;
            auto mouse_memory = mm.get_memory("pick_data");
            buffer_infos[0].setBuffer(mouse_memory.buffer).setOffset(0u).setRange(vk::WholeSize);

            std::array<vk::WriteDescriptorSet, 1> write_descriptor_sets;
            write_descriptor_sets[0].setDstSet(*gpu.descriptor_set).setDstBinding(2).setDstArrayElement(0)
                                    .setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eStorageBuffer)
                                    .setBufferInfo(buffer_infos[0]);

            gpu.device.updateDescriptorSets(write_descriptor_sets, nullptr);
        }

    }

    void ThreeD::update(vk::raii::CommandBuffer &command_buffer)
    {
        /* Clear Memory */
        {
            auto &memory_manager = Core::get_instance().mm;
            auto pick_mem = memory_manager.get_memory("pick_data");
            std::memset(pick_mem.alloc_info.pMappedData, 0, sizeof(PickData));
        }

        /* Render objects */
        for (auto display_object : display_objects_)
        {
            display_object->update(command_buffer);

            auto pickable = std::dynamic_pointer_cast<BasePickable>(display_object);
            if (pickable && pickable->display_aabb())
            {
                auto base_transfrom = std::dynamic_pointer_cast<BaseTransform>(display_object);
                auto transform = base_transfrom->get_transform();
                aabb_.set_transform(transform);
                aabb_.set_box(pickable->box());
                aabb_.render(command_buffer);
            }
        }
    }

    std::shared_ptr<BaseDisplayObject> ThreeD::pick(const Eigen::Vector2d &uv)
    {    
        update_pick_data();
        
        auto origin = camera_.uv_to_near_xyz(uv);
        auto direction = camera_.uv_to_direction(uv);

        double min_dist = std::numeric_limits<double>::max();
        std::shared_ptr<BasePickable> picked;

        for (auto &object : display_objects_)
        {
            auto pickable = std::dynamic_pointer_cast<BasePickable>(object);
            if (pickable)
            {
                auto dist = pickable->pick(origin, direction);
                if (0 < dist && dist < min_dist)
                {
                    picked = pickable;
                    min_dist = dist;
                }
            }
        }

        return std::dynamic_pointer_cast<BaseDisplayObject>(picked);
    }

    void ThreeD::add(std::shared_ptr<BaseDisplayObject> display_object)
    {
        display_objects_.push_back(display_object);
    }

    std::optional<size_t> ThreeD::peek(std::shared_ptr<BaseDisplayObject> display_object)
    {
        size_t ret = 0;
        for (auto &object : display_objects_)
        {
            if (object == display_object)
            {
                return ret;
            }
            ret++;
        }
        return std::nullopt;
    }

    std::shared_ptr<BaseDisplayObject> ThreeD::get_display_object(const size_t &index)
    {
        assert(index < display_objects_.size());
        return display_objects_[index];
    }

    void ThreeD::erase(const size_t &index)
    {
        assert(index < display_objects_.size());
        display_objects_.erase(display_objects_.begin() + index);
    }

    void ThreeD::erase(std::shared_ptr<BaseDisplayObject> display_object)
    {
        auto it = remove_if(display_objects_.begin(), display_objects_.end(),
                            [&](std::shared_ptr<BaseDisplayObject> &x)
                            { return x == display_object; });
        if (it != display_objects_.end())
        {
            auto r = distance(it, display_objects_.end());
            display_objects_.erase(it, display_objects_.end());
        }
    }

    Camera &ThreeD::camera()
    {
        return camera_;
    }

    const Camera &ThreeD::camera() const
    {
        return camera_;
    }

    void ThreeD::update_pick_data()
    {
        Core::get_instance().gpu.device.waitIdle();
        auto &memory_manager = Core::get_instance().mm;
        auto pick_mem = memory_manager.get_memory("pick_data");
        std::memcpy(&pick_data_, pick_mem.alloc_info.pMappedData, sizeof(PickData));
         spdlog::info("{} {} {} {}", pick_data_.instance, pick_data_.type, pick_data_.vertex, pick_data_.depth);
    }

    ThreeD::PickData ThreeD::get_pick_data() const
    {
        return pick_data_;
    }

}