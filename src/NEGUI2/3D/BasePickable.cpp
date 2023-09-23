#include "NEGUI2/3D/BasePickable.hpp"

namespace NEGUI2
{
    BasePickable::BasePickable()
        : display_aabb_(false)
    {
    }

    BasePickable::~BasePickable()
    {
    }

    bool BasePickable::display_aabb() const
    {
        return display_aabb_;
    }

    void BasePickable::set_display_aabb(const bool aabb)
    {
        display_aabb_ = aabb;
    }

    void BasePickable::toggle_display_aabb()
    {
        display_aabb_ = !display_aabb_;
    }

    Eigen::AlignedBox3d BasePickable::box() const
    {
        return box_;
    }
}