#ifndef _BASE_TRANSFORM_HPP
#define _BASE_TRANSFORM_HPP
#include <cinttypes>
#include <Eigen/Dense>

namespace NEGUI2
{
    class BaseTransform
    {
    protected:
        Eigen::Affine3d transform_;

    public:
        BaseTransform();
        virtual ~BaseTransform();

        Eigen::Affine3d get_transform() const;
        void set_transform(const Eigen::Affine3d& transform);
        Eigen::Vector3d get_position() const;
        void set_position(const Eigen::Vector3d& position);
        Eigen::Matrix3d get_orientation() const;
        void set_orientation(const Eigen::Matrix3d& rotation);
        Eigen::Vector3d front() const;
        Eigen::Vector3d right() const;
        Eigen::Vector3d up() const;

    };
}

#endif