#include "NEGUI2/Core/Window.hpp"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

namespace NEGUI2
{
  Window::Window()
  {

  }

  void Window::init()
  {
    spdlog::info("Initializing Window");
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window_ = glfwCreateWindow(WIDTH, HEIGHT, "NEGUI2", nullptr, nullptr);
  }

  Window::~Window()
  {
    // spdlog::info("Finalizing Window");
    glfwDestroyWindow(window_);
    glfwTerminate();
  }

  GLFWwindow *Window::get_raw()
  {
    return window_;
  }

  bool Window::should_close() const
  {
    return glfwWindowShouldClose(window_);
  }

  void Window::get_extent(int& width, int& height) const
  {
    glfwGetFramebufferSize(window_, &width, &height);
  }

  vk::Extent2D Window::get_extent() const
  {
    int width, height;
    glfwGetFramebufferSize(window_, &width, &height);
    vk::Extent2D extent{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    return extent;
  }
}
