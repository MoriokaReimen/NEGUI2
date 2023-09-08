#include "NEGUI2/3D/Triangle.hpp"
#include <typeinfo>

namespace NEGUI2
{
        uint32_t Triangle::instance_count_ = 0u;

        void Triangle::init()
        {
            instance_count_++;
            instance_id_ = instance_count_;
        }

        void Triangle::destroy()
        {

        }
        
        void Triangle::update(vk::raii::CommandBuffer& command)
        {

        }

        uint32_t Triangle::get_type_id()
        {
            auto& id = typeid(Triangle);
            return static_cast<uint32_t>(id.hash_code());
        }

        uint32_t Triangle::get_instance_id()
        {
            return instance_id_;
        }
}