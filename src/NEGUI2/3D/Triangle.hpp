#ifndef _TRIANGLE_HPP
#define _TRIANGLE_HPP
#include "NEGUI2/3D/IDisplayObject.hpp"


namespace NEGUI2
{
    class Triangle : public IDisplayObject
    {
        static uint32_t instance_count_;
        uint32_t instance_id_;
        vk::raii::Pipeline pipeline_;
        vk::raii::PipelineLayout pipeline_layout_;

        std::array<float, 3 * 3> vertex_data_;

        public:
        Triangle();
        ~Triangle() override;

        void init() override;
        void destroy() override;
        void update(vk::raii::CommandBuffer &command) override;
        void rebuild() override;
        uint32_t get_type_id() override;
        uint32_t get_instance_id() override;
    };

}
#endif