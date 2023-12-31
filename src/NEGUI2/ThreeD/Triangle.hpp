#ifndef _BASE_HPP
#define _BASE_HPP
#include "NEGUI2/ThreeD/BaseDisplayObject.hpp"
#include "NEGUI2/ThreeD/BaseTransform.hpp"
#include <Eigen/Dense>

namespace NEGUI2
{
    class Triangle : public BaseDisplayObject, public BaseTransform
    {
        static int32_t instance_count_;
        PushConstant push_constant_;
        int32_t instance_id_;
        vk::raii::Pipeline pipeline_;
        vk::raii::PipelineLayout pipeline_layout_;

        std::array<Eigen::Vector3f, 3> vertex_data_;
        std::array<Eigen::Vector4f, 3> color_data_;

        public:
        Triangle();
        ~Triangle() override;

        void init() override;
        void destroy() override;
        void update(vk::raii::CommandBuffer &command) override;
        void rebuild() override;
        int32_t get_type_id() override;
        int32_t get_instance_id() override;
    };

}
#endif