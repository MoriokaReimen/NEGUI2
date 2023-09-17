#include "Widget.hpp"

namespace App
{
    Widget::Widget(std::shared_ptr<entt::registry> registry)
        : IModule(registry)
    {
    }

    Widget::~Widget()
    {
    }

    void Widget::init()
    {
    }

    void Widget::update()
    {
    }

}
