#include "NEGUI2/ThreeD/BaseDisplayObject.hpp"

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
    
    void BaseDisplayObject::set_enable(const bool enable)
    {
        enable_ = enable;
    }
}