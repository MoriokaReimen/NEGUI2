#ifndef _LINE_HPP
#define _LINE_HPP
#include "NEGUI2/ThreeD/BaseDisplayObject.hpp"
#include "NEGUI2/ThreeD/BaseTransform.hpp"
#include <Eigen/Dense>

namespace NEGUI2
{
    class Line : public BaseDisplayObject, BaseTransform
    {
    public:
        struct PointData
        {
            Eigen::Vector3f start;
            Eigen::Vector3f end;
            Eigen::Vector4f color;
            float diameter;
        };

    private:
        static uint32_t instance_count_;
        vk::raii::Pipeline pipeline_;
        vk::raii::PipelineLayout pipeline_layout_;
        PushConstant push_constant_;

        std::vector<PointData> line_data_;

    public:
        Line();
        ~Line() override;

        void init() override;
        void destroy() override;
        void update(vk::raii::CommandBuffer &command) override;
        void rebuild() override;
        uint32_t get_type_id() override;
        uint32_t get_instance_id() override;

        bool add(const Eigen::Vector3f &start, const Eigen::Vector3f &end,
                 const Eigen::Vector4f &color = Eigen::Vector4f::UnitW(), const float &diameter = 2);
        bool popback();
        PointData get(size_t index) const;
        size_t size() const;
    };
}
#endif