#include "NEGUI2/3D/IDisplayObject.hpp"

namespace NEGUI2
{
    IDisplayObject::IDisplayObject()
    : enable_(true), show_aabb_(false)
    {
    }

    IDisplayObject::~IDisplayObject()
    {
    }

    bool IDisplayObject::is_enable() const
    {
        return enable_;
    }
    
    void IDisplayObject::set_enabel(const bool enable)
    {
        enable_ = enable;
    }

    bool IDisplayObject::show_aabb() const
    {
        return show_aabb_;
    }

    void IDisplayObject::set_aabb(const bool aabb)
    {
        show_aabb_ = aabb;
    }

}