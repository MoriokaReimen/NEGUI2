#ifndef _IUSER_INTERFACE_HPP
#define _IUSER_INTERFACE_HPP
namespace NEGUI2
{
    class IUserInterface
    {
    protected:
        bool is_active_;

    public:
        IUserInterface() : is_active_(true) {}
        virtual ~IUserInterface() {}
        virtual void update() = 0;

        bool is_active() const
        {
            return is_active_;
        }
        void set_active(const bool &is_active = true)
        {
            is_active_ = is_active;
        }
    };
}
#endif