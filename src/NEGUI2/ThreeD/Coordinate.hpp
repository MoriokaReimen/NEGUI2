#ifndef _COORDINATE_HPP
#define _COORDINATE_HPP
#include "NEGUI2/ThreeD/BaseDisplayObject.hpp"
#include "NEGUI2/ThreeD/BaseTransform.hpp"
#include "NEGUI2/ThreeD/BasePickable.hpp"
#include <Eigen/Dense>

namespace NEGUI2
{
    class Coordinate : public BaseDisplayObject, public BaseTransform, public BasePickable
    {
        static int32_t instance_count_;
        PushConstant push_constant_;
        vk::raii::Pipeline pipeline_;
        vk::raii::PipelineLayout pipeline_layout_;

        std::array<Eigen::Vector3f, 6> vertex_data_;
        std::array<Eigen::Vector4f, 6> color_data_;

        public:
        Coordinate();
        ~Coordinate() override;

        void init() override;
        void destroy() override;
        void update(vk::raii::CommandBuffer &command) override;
        void rebuild() override;
        int32_t get_type_id() override;
        int32_t get_instance_id() override;

        double pick(const Eigen::Vector3d& origin, const Eigen::Vector3d& direction) override;
    };

}
#endif