#ifndef _SCENE_HPP
#define _SCENE_HPP
#include "IModule.hpp"
#include "NEGUI2/3D/Camera.hpp"
#include "NEGUI2/3D/IDisplayObject.hpp"


namespace App
{
    class Scene : public IModule
    {
        NEGUI2::Camera camera_;
        void handle_camera_();
        public:
        Scene(std::shared_ptr<entt::registry> registry);
        ~Scene() override;
        void init() override;
        void update() override;
    };

}
#endif