#pragma once
#include "utils.hpp"
#include "GLFW/glfw3.h"
#include "vulkan/vulkan_raii.hpp"

#include <vector>
#include <string>
#include "thing.hpp"




class App
{
public:
    void run();

private:
    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();

    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFrameBuffers();
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createCommandPool();
    void createVertexBuffer();
    void createUniformBuffer();
    void createDescriptorPool();
    void createDescriptorSet();
    void createCommandBuffer();
    void createSyncObjects();

    void drawFrame();
    void updateUniformBuffer();
    void processInput();
    void createObjects();

    void recordCommandBuffer(uint32_t imageIndex);
    void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
                      vk::raii::Buffer &buffer, vk::raii::DeviceMemory &bufferMemory);
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;
    static std::vector<char> readFile(const std::string &filename);
    std::vector<const char *> getRequiredInstanceExtensions();
    [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char> &code) const;
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *);
    bool isDeviceSuitable(vk::raii::PhysicalDevice const &physicalDevice);
    vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities);
    static uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);
    static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &availableFormats);
    static vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes);

    GLFWwindow *window;
    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
    vk::raii::PhysicalDevice physicalDevice = nullptr;
    vk::raii::Device device = nullptr;
    vk::raii::Queue queue = nullptr;
    vk::raii::RenderPass renderPass = nullptr;
    std::vector<vk::raii::Framebuffer> swapChainFramebuffers;
    vk::raii::SurfaceKHR surface = nullptr;
    vk::raii::SwapchainKHR swapChain = nullptr;
    std::vector<vk::Image> swapChainImages;
    vk::SurfaceFormatKHR swapChainSurfaceFormat;
    vk::Extent2D swapChainExtent;
    std::vector<vk::raii::ImageView> swapChainImageViews;
    vk::raii::Image depthImage = nullptr;
    vk::raii::DeviceMemory depthImageMemory = nullptr;
    vk::raii::ImageView depthImageView = nullptr;
    vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
    vk::raii::PipelineLayout pipelineLayout = nullptr;
    vk::raii::Pipeline graphicsPipeline = nullptr;
    vk::raii::CommandPool commandPool = nullptr;
    vk::raii::Buffer vertexBuffer = nullptr;
    vk::raii::DeviceMemory vertexBufferMemory = nullptr;
    vk::raii::Buffer uniformBuffer = nullptr;
    vk::raii::DeviceMemory uniformBufferMemory = nullptr;
    void *uniformBufferMapped = nullptr;
    vk::raii::DescriptorPool descriptorPool = nullptr;
    vk::raii::DescriptorSet descriptorSet = nullptr;
    vk::raii::CommandBuffer commandBuffer = nullptr;
    uint32_t queueIndex = ~0;
    vk::raii::Semaphore presentCompleteSemaphore = nullptr;
    vk::raii::Semaphore renderFinishedSemaphore = nullptr;
    vk::raii::Fence drawFence = nullptr;
    std::vector<const char *> requiredDeviceExtension = {vk::KHRSwapchainExtensionName};
    vk::DeviceSize slotSize;

    int numberOfObjects;
    std::vector<std::unique_ptr<Thing>> objects;
    std::vector<glm::mat4> modelMatrices{};

    std::vector<Vertex> vertices;

    glm::vec3 cameraPos{0.0f, -1.0f, 0.0f};
    glm::vec3 cameraFront{0.0f, 1.0f, 0.0f};
    glm::vec3 cameraUp{0.0f, 0.0f, 1.0f};
    float yaw{glm::radians(90.0f)};
    float pitch{0.0f};
    float lastFrameTime{0.0f};
    double lastMouseX{0.0};
    double lastMouseY{0.0};
};
