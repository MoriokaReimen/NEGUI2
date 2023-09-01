#include "NEGUI2/3D/BaseTransform.hpp"

namespace NEGUI2
{
    BaseTransform::BaseTransform()
        : transform_(Eigen::Affine3d::Identity())
    {
    }

    BaseTransform::~BaseTransform()
    {
    }

    Eigen::Affine3d BaseTransform::get_transform() const
    {
        return transform_;
    }

    void BaseTransform::set_transform(const Eigen::Affine3d &transform)
    {
        transform_ = transform;
    }

    Eigen::Vector3d BaseTransform::get_position() const
    {
        return transform_.translation();
    }

    void BaseTransform::set_position(const Eigen::Vector3d &position)
    {
        transform_.translation() = position;
    }

    Eigen::Matrix3d BaseTransform::get_orientation() const
    {
        return transform_.rotation();
    }

    void BaseTransform::set_orientation(const Eigen::Matrix3d &rotation)
    {
        /* reset rotation */
        auto inv = transform_.rotation();
        transform_ = rotation * inv.inverse() * transform_;
    }
}