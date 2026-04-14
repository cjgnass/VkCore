cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20) 

project(VkCore)

find_package(Vulkan REQUIRED)
add_subdirectory(vendor/glfw)
find_program(SLANGC slangc HINTS $ENV{VULKAN_SDK}/Bin $ENV{VULKAN_SDK}/bin)

set(SHADER_SRC  ${CMAKE_SOURCE_DIR}/shader.slang)
set(SHADER_OUT  ${CMAKE_SOURCE_DIR}/shaders/slang.spv)

add_custom_command(
    OUTPUT  ${SHADER_OUT}
    COMMAND ${SLANGC} ${SHADER_SRC} -target spirv -o ${SHADER_OUT}
    DEPENDS ${SHADER_SRC}
    COMMENT "Compiling Slang shader -> SPIR-V"
    VERBATIM
)

add_custom_target(shaders DEPENDS ${SHADER_OUT})







add_executable(main main.cpp app.cpp)
target_link_libraries(main PRIVATE glfw Vulkan::Vulkan)
target_include_directories(main PRIVATE include)
add_dependencies(main shaders)


