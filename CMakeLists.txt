cmake_minimum_required(VERSION 3.27)
project(Sample)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


##################################################
# Configure CMake Modules
##################################################
include(${CMAKE_CURRENT_LIST_DIR}/cmake/External.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/Shader.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/RuntimeResource.cmake)

##################################################
# Configure NEGUI2 Library
##################################################
file(GLOB NEGUI2_SRC ${CMAKE_CURRENT_LIST_DIR}/src/NEGUI2/*.cpp)
add_library(NEGUI2 ${NEGUI2_SRC})
target_include_directories(NEGUI2 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)
target_link_libraries(NEGUI2 PUBLIC glfw spdlog::spdlog Vulkan::Headers Vulkan::Vulkan glm::glm imgui::imgui
                                    glslang glslang-default-resource-limits SPIRV VulkanMemoryAllocator glad_vulkan_13)
target_glsl_shaders(NEGUI2 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/shader/base.frag ${CMAKE_CURRENT_LIST_DIR}/shader/base.vert)

##################################################
# Configure Sample Executable
##################################################
file(GLOB SAMPLE_SRC ${CMAKE_CURRENT_LIST_DIR}/Sample/*.cpp)
add_executable(Sample ${SAMPLE_SRC})
target_link_libraries(Sample PRIVATE NEGUI2)

##################################################
# Configure Test Executable
##################################################
file(GLOB TEST_SRC ${CMAKE_CURRENT_LIST_DIR}/test/*.cpp)
add_executable(Test ${TEST_SRC})
target_link_libraries(Test PRIVATE GTest::gtest_main)
