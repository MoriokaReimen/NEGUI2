#ifndef _UI_DEMO_HPP
#define _UI_DEMO_HPP
#include "NEGUI2/IUserInterface.hpp"

namespace NEGUI2
{
    class UiDemo : public IUserInterface
    {

    public:
        UiDemo();
        virtual ~UiDemo() override;
        virtual void update() override;
    };

}
#endif