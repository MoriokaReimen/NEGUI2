#include "NEGUI2/ThreeD/BaseTransform.hpp"

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

    Eigen::Vector3d BaseTransform::front() const
    {
        auto camera_inv = transform_.rotation();
        camera_inv.inverse();
        auto front = camera_inv * Eigen::Vector3d(0.0, 0.0, 1.0);
        return front.normalized();
    }

    Eigen::Vector3d BaseTransform::right() const
    {
        auto camera_inv = transform_.rotation();
        camera_inv.inverse();
        auto right = camera_inv * Eigen::Vector3d(1.0, 0.0, 0.0);
        return right.normalized();
    }

    Eigen::Vector3d BaseTransform::up() const
    {
        auto camera_inv = transform_.rotation();
        camera_inv.inverse();
        auto up = camera_inv * Eigen::Vector3d(0.0, 1.0, 0.0);
        return up.normalized();
    }

}