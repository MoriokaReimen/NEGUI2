#ifndef _WINDOW_H
#define _WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <volk.h>
#include <cstdint>

namespace NEGUI2
{

class Window
{
  const int WIDTH = 800;
  const int HEIGHT = 600;
  GLFWwindow* window_;
  public:
    Window();
    ~Window();
    GLFWwindow* get_window();

    bool should_close() const;
    void get_extent(int& width, int& height) const;
};

}
#endif
