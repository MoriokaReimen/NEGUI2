#ifndef _TEXT_EDIT_DEMO_HPP
#define _TEXT_EDIT_DEMO_HPP
#include "NEGUI2/IUserInterface.hpp"
#include <TextEditor.h>
#include <string>

namespace NEGUI2
{
    class TextEditDemo : public IUserInterface
    {
        TextEditor editor_;
        std::string file_to_edit_;
    public:
        TextEditDemo();
        virtual ~TextEditDemo() override;
        virtual void update() override;
    };

}

#endif