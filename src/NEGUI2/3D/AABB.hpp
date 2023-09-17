#ifndef _AABB_HPP
#define _AABB_HPP
#include "NEGUI2/3D/IDisplayObject.hpp"
#include "NEGUI2/3D/BaseTransform.hpp"
#include <Eigen/Dense>

namespace NEGUI2
{
    class AABB : public BaseTransform
    {
        vk::raii::Pipeline pipeline_;
        vk::raii::PipelineLayout pipeline_layout_;
        Eigen::AlignedBox3d box_;

        public:
        AABB();
        ~AABB() override;

        void init();
        void set_box(const Eigen::AlignedBox3d& box);
        void render(vk::raii::CommandBuffer &command);
    };

}
#endif