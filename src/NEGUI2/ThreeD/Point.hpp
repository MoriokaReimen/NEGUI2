#ifndef _Point_HPP
#define _Point_HPP
#include "NEGUI2/ThreeD/BaseDisplayObject.hpp"
#include "NEGUI2/ThreeD/BaseTransform.hpp"
#include <Eigen/Dense>

namespace NEGUI2
{
    class Point : public BaseDisplayObject, BaseTransform
    {
    public:
        struct PointData
        {
            Eigen::Vector3f position;
            Eigen::Vector4f color;
            float diameter;
        };

    private:
        static int32_t instance_count_;
        vk::raii::Pipeline pipeline_;
        vk::raii::PipelineLayout pipeline_layout_;
        PushConstant push_constant_;

        std::vector<PointData> point_data_;

    public:
        Point();
        ~Point() override;

        void init() override;
        void destroy() override;
        void update(vk::raii::CommandBuffer &command) override;
        void rebuild() override;
        int32_t get_type_id() override;
        int32_t get_instance_id() override;

        bool add(const Eigen::Vector3f &position, const Eigen::Vector4f &color = Eigen::Vector4f::UnitW(), const float &diameter = 2);
        bool popback();
        PointData get(size_t index) const;
        size_t size() const;
    };
}
#endif