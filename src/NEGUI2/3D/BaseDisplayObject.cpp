#include "NEGUI2/3D/BaseDisplayObject.hpp"

namespace NEGUI2
{
    BaseDisplayObject::BaseDisplayObject()
    : enable_(true)
    {
    }

    BaseDisplayObject::~BaseDisplayObject()
    {
    }

    bool BaseDisplayObject::is_enable() const
    {
        return enable_;
    }
    
    void BaseDisplayObject::set_enabel(const bool enable)
    {
        enable_ = enable;
    }
}