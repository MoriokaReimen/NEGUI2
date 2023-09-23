#ifndef _FullShader_HPP
#define _FullShader_HPP
#include "NEGUI2/3D/IDisplayObject.hpp"
#include "NEGUI2/3D/BaseTransform.hpp"
#include <Eigen/Dense>

namespace NEGUI2
{
    class FullShader : public IDisplayObject, public BaseTransform
    {
        static uint32_t instance_count_;
        PushConstant push_constant_;
        vk::raii::Pipeline pipeline_;
        vk::raii::PipelineLayout pipeline_layout_;

        public:
        FullShader();
        ~FullShader() override;

        void init() override;
        void destroy() override;
        void update(vk::raii::CommandBuffer &command) override;
        void rebuild() override;
        uint32_t get_type_id() override;
        uint32_t get_instance_id() override;
    };

}
#endif