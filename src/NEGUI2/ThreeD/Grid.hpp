#ifndef _GRID_HPP
#define _GRID_HPP
#include "NEGUI2/ThreeD/BaseDisplayObject.hpp"
#include "NEGUI2/ThreeD/BaseTransform.hpp"
#include <Eigen/Dense>

namespace NEGUI2
{
    class Grid : public BaseDisplayObject, public BaseTransform
    {
        static int32_t instance_count_;
        PushConstant push_constant_;
        vk::raii::Pipeline pipeline_;
        vk::raii::PipelineLayout pipeline_layout_;

        public:
        Grid();
        ~Grid() override;

        void init() override;
        void destroy() override;
        void update(vk::raii::CommandBuffer &command) override;
        void rebuild() override;
        int32_t get_type_id() override;
        int32_t get_instance_id() override;
    };

}
#endif