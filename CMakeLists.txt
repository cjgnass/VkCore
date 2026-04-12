cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20) 

project(VkCore)

find_package(Vulkan REQUIRED)
add_subdirectory(vendor/glfw)

add_executable(main main.cpp app.cpp)
target_link_libraries(main PRIVATE glfw Vulkan::Vulkan)
target_include_directories(main PRIVATE include)


