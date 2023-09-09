#ifndef _WINDOW_H
#define _WINDOW_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <cstdint>

namespace NEGUI2
{

  class Window
  {
    friend class Core;
    const int WIDTH = 800;
    const int HEIGHT = 600;
    GLFWwindow *window_;
    Window();
    void init();

    Window(const Window& other) = delete;
    Window& operator=(const Window& other) = delete;

  public:
    ~Window();
    GLFWwindow *get_window();
    bool should_close() const;
    void get_extent(int &width, int &height) const;
    vk::Extent2D get_extent() const;
  };

}
#endif
