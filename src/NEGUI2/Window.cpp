#include "NEGUI2/Window.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

namespace NEGUI2
{
  Window::Window()
  {
    spdlog::info("Initializing Window");
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window_ = glfwCreateWindow(WIDTH, HEIGHT, "NEGUI2", nullptr, nullptr);
  }

  Window::~Window()
  {
    spdlog::info("Finalizing Window");
    glfwDestroyWindow(window_);
    glfwTerminate();
  }

  GLFWwindow *Window::get_window()
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
}
