#include "app.hpp"
#include "utils.hpp"
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <ranges>
#include <vector>

constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 720;

std::vector<Vertex> s1 = createQuad(0, 0, 0.25, 0.25);

std::vector<std::vector<Vertex>> shapes{s1};

const std::vector<Vertex> vertices = shapesToVertices(shapes);
const std::vector<char const *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

std::vector<const char *> App::getRequiredInstanceExtensions()
{
  uint32_t glfwExtensionCount = 0;
  auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
  if (enableValidationLayers)
  {
    extensions.push_back(vk::EXTDebugUtilsExtensionName);
  }
  return extensions;
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL
App::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                   vk::DebugUtilsMessageTypeFlagsEXT type,
                   const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
                   void *pUserData)
{
  std::cerr << "validation layer: type " << to_string(type)
            << " msg: " << pCallbackData->pMessage << std::endl;
  return vk::False;
}

void App::run()
{
  initWindow();
  initVulkan();
  mainLoop();
  cleanup();
}

void App::initWindow()
{
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  window = glfwCreateWindow(WIDTH, HEIGHT, "VkCore", nullptr, nullptr);
}

void App::initVulkan()
{
  createInstance();
  setupDebugMessenger();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  createSwapChain();
  createImageViews();
  createDepthResources();
  createRenderPass();
  createFrameBuffers();
  createDescriptorSetLayout();
  createGraphicsPipeline();
  createCommandPool();
  createVertexBuffer();
  createUniformBuffer();
  createDescriptorPool();
  createDescriptorSet();
  createCommandBuffer();
  createSyncObjects();
}

void App::mainLoop()
{
  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();
    drawFrame();
  }
  device.waitIdle();
}

void App::cleanup()
{
  glfwDestroyWindow(window);
  glfwTerminate();
}

void App::drawFrame()
{
  auto fenceResult = device.waitForFences(*drawFence, vk::True, UINT64_MAX);
  if (fenceResult != vk::Result::eSuccess)
  {
    throw std::runtime_error("failed to wait for fence!");
  }
  device.resetFences(*drawFence);

  auto [result, imageIndex] = swapChain.acquireNextImage(
      UINT64_MAX, *presentCompleteSemaphore, nullptr);

  updateUniformBuffer();
  recordCommandBuffer(imageIndex);

  queue.waitIdle(); // NOTE: for simplicity, wait for the queue to be idle
                    // before starting the frame In the next chapter you see how
                    // to use multiple frames in flight and fences to sync

  vk::PipelineStageFlags waitDestinationStageMask(
      vk::PipelineStageFlagBits::eColorAttachmentOutput);
  vk::SubmitInfo submitInfo{};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &*presentCompleteSemaphore;
  submitInfo.pWaitDstStageMask = &waitDestinationStageMask;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &*commandBuffer;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &*renderFinishedSemaphore;
  queue.submit(submitInfo, *drawFence);

  vk::PresentInfoKHR presentInfoKHR{};
  presentInfoKHR.waitSemaphoreCount = 1;
  presentInfoKHR.pWaitSemaphores = &*renderFinishedSemaphore;
  presentInfoKHR.swapchainCount = 1;
  presentInfoKHR.pSwapchains = &*swapChain;
  presentInfoKHR.pImageIndices = &imageIndex;
  result = queue.presentKHR(presentInfoKHR);
  switch (result)
  {
  case vk::Result::eSuccess:
    break;
  case vk::Result::eSuboptimalKHR:
    std::cout
        << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !\n";
    break;
  default:
    break; // an unexpected result is returned!
  }
}

void App::setupDebugMessenger()
{
  if (!enableValidationLayers)
    return;

  vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
  vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
      vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
      vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
  vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{};
  debugUtilsMessengerCreateInfoEXT.messageSeverity = severityFlags;
  debugUtilsMessengerCreateInfoEXT.messageType = messageTypeFlags,
  debugUtilsMessengerCreateInfoEXT.pfnUserCallback = &debugCallback;
  debugMessenger =
      instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
}

void App::createInstance()
{
  vk::ApplicationInfo appInfo{};
  appInfo.pApplicationName = "Hello Triangle";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = vk::ApiVersion12;
  std::vector<char const *> requiredLayers;
  if (enableValidationLayers)
  {
    requiredLayers.assign(validationLayers.begin(), validationLayers.end());
  }

  // Check if the required layers are supported by the Vulkan implementation.
  auto layerProperties = context.enumerateInstanceLayerProperties();
  auto unsupportedLayerIt = std::ranges::find_if(
      requiredLayers, [&layerProperties](auto const &requiredLayer)
      { return std::ranges::none_of(
            layerProperties, [requiredLayer](auto const &layerProperty)
            { return strcmp(layerProperty.layerName, requiredLayer) == 0; }); });
  if (unsupportedLayerIt != requiredLayers.end())
  {
    throw std::runtime_error("Required layer not supported: " +
                             std::string(*unsupportedLayerIt));
  }

  // Get the required extensions.
  auto requiredExtensions = getRequiredInstanceExtensions();

  // Check if the required extensions are supported by the Vulkan
  // implementation.
  auto extensionProperties = context.enumerateInstanceExtensionProperties();
  auto unsupportedPropertyIt = std::ranges::find_if(
      requiredExtensions,
      [&extensionProperties](auto const &requiredExtension)
      {
        return std::ranges::none_of(
            extensionProperties,
            [requiredExtension](auto const &extensionProperty)
            {
              return strcmp(extensionProperty.extensionName,
                            requiredExtension) == 0;
            });
      });
  if (unsupportedPropertyIt != requiredExtensions.end())
  {
    throw std::runtime_error("Required extension not supported: " +
                             std::string(*unsupportedPropertyIt));
  }

  vk::InstanceCreateInfo createInfo{};
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
  createInfo.ppEnabledLayerNames = requiredLayers.data();
  createInfo.enabledExtensionCount =
      static_cast<uint32_t>(requiredExtensions.size());
  createInfo.ppEnabledExtensionNames = requiredExtensions.data();
  instance = vk::raii::Instance(context, createInfo);
}

void App::pickPhysicalDevice()
{
  std::vector<vk::raii::PhysicalDevice> physicalDevices =
      instance.enumeratePhysicalDevices();
  auto const devIter =
      std::ranges::find_if(physicalDevices, [&](auto const &physicalDevice)
                           { return isDeviceSuitable(physicalDevice); });
  if (devIter == physicalDevices.end())
  {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
  physicalDevice = *devIter;
}

bool App::isDeviceSuitable(vk::raii::PhysicalDevice const &physicalDevice)
{
  // Check if the physicalDevice supports the Vulkan 1.3 API version
  auto requiredApiVersion = VK_API_VERSION_1_0;
  auto apiVersion = physicalDevice.getProperties().apiVersion;
  bool supportsVulkan1_0 =
      physicalDevice.getProperties().apiVersion >= VK_API_VERSION_1_0;

  // Check if any of the queue families support graphics operations
  auto queueFamilies = physicalDevice.getQueueFamilyProperties();
  bool supportsGraphics =
      std::ranges::any_of(queueFamilies, [](auto const &qfp)
                          { return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });

  // Check if all required physicalDevice extensions are available
  auto availableDeviceExtensions =
      physicalDevice.enumerateDeviceExtensionProperties();
  bool supportsAllRequiredExtensions = std::ranges::all_of(
      requiredDeviceExtension,
      [&availableDeviceExtensions](auto const &requiredDeviceExtension)
      {
        return std::ranges::any_of(
            availableDeviceExtensions,
            [requiredDeviceExtension](auto const &availableDeviceExtension)
            {
              return strcmp(availableDeviceExtension.extensionName,
                            requiredDeviceExtension) == 0;
            });
      });

  // Check if the physicalDevice supports the required features
  auto features = physicalDevice.template getFeatures2<
      vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,
      vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
  bool supportsRequiredFeatures =
      features.template get<vk::PhysicalDeviceVulkan11Features>()
          .shaderDrawParameters &&
      features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>()
          .extendedDynamicState;

  // Return true if the physicalDevice meets all the criteria
  return supportsVulkan1_0 && supportsGraphics &&
         supportsAllRequiredExtensions && supportsRequiredFeatures;
}

void App::createLogicalDevice()
{
  std::vector<vk::QueueFamilyProperties> queueFamilyProperties =
      physicalDevice.getQueueFamilyProperties();
  auto graphicsQueueFamilyProperty =
      std::ranges::find_if(queueFamilyProperties, [](auto const &qfp)
                           { return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) !=
                                    static_cast<vk::QueueFlags>(0); });
  assert(graphicsQueueFamilyProperty != queueFamilyProperties.end() &&
         "No graphics queue family found!");
  auto graphicsIndex = static_cast<uint32_t>(std::distance(
      queueFamilyProperties.begin(), graphicsQueueFamilyProperty));
  queueIndex = graphicsIndex;
  vk::PhysicalDeviceFeatures2 vkPhysicalDeviceFeatures2{};
  vk::PhysicalDeviceVulkan11Features vkPhysicalDeviceVulkan11Features{};
  vkPhysicalDeviceVulkan11Features.shaderDrawParameters = true;
  vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
      vkPhysicalDeviceExtendedDynamicStateFeaturesEXT{};
  vkPhysicalDeviceExtendedDynamicStateFeaturesEXT.extendedDynamicState = true;
  vk::StructureChain<vk::PhysicalDeviceFeatures2,
                     vk::PhysicalDeviceVulkan11Features,
                     vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
      featureChain{vkPhysicalDeviceFeatures2, vkPhysicalDeviceVulkan11Features,
                   vkPhysicalDeviceExtendedDynamicStateFeaturesEXT};
  float queuePriority = 0.5f;
  vk::DeviceQueueCreateInfo deviceQueueCreateInfo{};
  deviceQueueCreateInfo.queueFamilyIndex = graphicsIndex;
  deviceQueueCreateInfo.queueCount = 1;
  deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
  vk::DeviceCreateInfo deviceCreateInfo{};
  deviceCreateInfo.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
  deviceCreateInfo.queueCreateInfoCount = 1;
  deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
  deviceCreateInfo.enabledExtensionCount =
      static_cast<uint32_t>(requiredDeviceExtension.size());
  deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtension.data();
  device = vk::raii::Device(physicalDevice, deviceCreateInfo);
  queue = vk::raii::Queue(device, graphicsIndex, 0);
}

void App::createSurface()
{
  VkSurfaceKHR _surface;
  if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != 0)
  {
    throw std::runtime_error("failed to create window surface!");
  }
  surface = vk::raii::SurfaceKHR(instance, _surface);
}

void App::createSwapChain()
{
  vk::SurfaceCapabilitiesKHR surfaceCapabilities =
      physicalDevice.getSurfaceCapabilitiesKHR(*surface);
  swapChainExtent = chooseSwapExtent(surfaceCapabilities);
  uint32_t minImageCount = chooseSwapMinImageCount(surfaceCapabilities);

  std::vector<vk::SurfaceFormatKHR> availableFormats =
      physicalDevice.getSurfaceFormatsKHR(*surface);
  swapChainSurfaceFormat = chooseSwapSurfaceFormat(availableFormats);

  std::vector<vk::PresentModeKHR> availablePresentModes =
      physicalDevice.getSurfacePresentModesKHR(*surface);
  vk::PresentModeKHR presentMode = chooseSwapPresentMode(availablePresentModes);

  vk::SwapchainCreateInfoKHR swapChainCreateInfo{};
  swapChainCreateInfo.surface = *surface,
  swapChainCreateInfo.minImageCount = minImageCount;
  swapChainCreateInfo.imageFormat = swapChainSurfaceFormat.format;
  swapChainCreateInfo.imageColorSpace = swapChainSurfaceFormat.colorSpace;
  swapChainCreateInfo.imageExtent = swapChainExtent;
  swapChainCreateInfo.imageArrayLayers = 1;
  swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
  swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
  swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
  swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
  swapChainCreateInfo.presentMode = presentMode;
  swapChainCreateInfo.clipped = true;
  swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
  swapChainImages = swapChain.getImages();
}

vk::Extent2D
App::chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities)
{
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max())
  {
    return capabilities.currentExtent;
  }
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  return {std::clamp<uint32_t>(width, capabilities.minImageExtent.width,
                               capabilities.maxImageExtent.width),
          std::clamp<uint32_t>(height, capabilities.minImageExtent.height,
                               capabilities.maxImageExtent.height)};
}

uint32_t App::chooseSwapMinImageCount(
    vk::SurfaceCapabilitiesKHR const &surfaceCapabilities)
{
  auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
  if ((0 < surfaceCapabilities.maxImageCount) &&
      (surfaceCapabilities.maxImageCount < minImageCount))
  {
    minImageCount = surfaceCapabilities.maxImageCount;
  }
  return minImageCount;
}

vk::SurfaceFormatKHR App::chooseSwapSurfaceFormat(
    std::vector<vk::SurfaceFormatKHR> const &availableFormats)
{
  assert(!availableFormats.empty());
  const auto formatIt =
      std::ranges::find_if(availableFormats, [](const auto &format)
                           { return format.format == vk::Format::eB8G8R8A8Srgb &&
                                    format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
  return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}

vk::PresentModeKHR App::chooseSwapPresentMode(
    std::vector<vk::PresentModeKHR> const &availablePresentModes)
{
  assert(std::ranges::any_of(availablePresentModes, [](auto presentMode)
                             { return presentMode == vk::PresentModeKHR::eFifo; }));
  return std::ranges::any_of(availablePresentModes,
                             [](const vk::PresentModeKHR value)
                             {
                               return vk::PresentModeKHR::eMailbox == value;
                             })
             ? vk::PresentModeKHR::eMailbox
             : vk::PresentModeKHR::eFifo;
}

void App::createImageViews()
{
  assert(swapChainImageViews.empty());

  vk::ImageViewCreateInfo imageViewCreateInfo{};
  imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
  imageViewCreateInfo.format = swapChainSurfaceFormat.format;
  imageViewCreateInfo.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1,
                                          0, 1};
  for (auto &image : swapChainImages)
  {
    imageViewCreateInfo.image = image;
    swapChainImageViews.emplace_back(device, imageViewCreateInfo);
  }
}

void App::createDepthResources()
{
  vk::Format depthFormat = vk::Format::eD32Sfloat;

  vk::ImageCreateInfo imageInfo{};
  imageInfo.imageType = vk::ImageType::e2D;
  imageInfo.format = depthFormat;
  vk::Extent3D vkExtent3D{swapChainExtent.width, swapChainExtent.height, 1};
  imageInfo.extent = vkExtent3D;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.samples = vk::SampleCountFlagBits::e1;
  imageInfo.tiling = vk::ImageTiling::eOptimal;
  imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
  imageInfo.initialLayout = vk::ImageLayout::eUndefined;
  depthImage = vk::raii::Image(device, imageInfo);

  vk::MemoryRequirements memRequirements = depthImage.getMemoryRequirements();
  vk::MemoryAllocateInfo allocInfo{};
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(
      memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
  depthImageMemory = vk::raii::DeviceMemory(device, allocInfo);
  depthImage.bindMemory(*depthImageMemory, 0);

  vk::ImageViewCreateInfo viewInfo{};
  viewInfo.image = *depthImage;
  viewInfo.viewType = vk::ImageViewType::e2D;
  viewInfo.format = depthFormat;
  viewInfo.subresourceRange = {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1};
  depthImageView = vk::raii::ImageView(device, viewInfo);
}

std::vector<char> App::readFile(const std::string &filename)
{
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open())
  {
    throw std::runtime_error("failed to open file!");
  }
  std::vector<char> buffer(file.tellg());
  file.seekg(0, std::ios::beg);
  file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
  file.close();
  return buffer;
}

void App::createFrameBuffers()
{
  for (auto &view : swapChainImageViews)
  {
    std::array<vk::ImageView, 2> attachments = {*view, *depthImageView};
    vk::FramebufferCreateInfo fbInfo{};
    fbInfo.renderPass = *renderPass;
    fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    fbInfo.pAttachments = attachments.data();
    fbInfo.width = swapChainExtent.width;
    fbInfo.height = swapChainExtent.height;
    fbInfo.layers = 1;
    swapChainFramebuffers.emplace_back(device, fbInfo);
  }
}

void App::createRenderPass()
{
  vk::AttachmentDescription colorAttachment{};
  colorAttachment.format = swapChainSurfaceFormat.format;
  colorAttachment.samples = vk::SampleCountFlagBits::e1;
  colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
  colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
  colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
  colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
  colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

  vk::AttachmentDescription depthAttachment{};
  depthAttachment.format = vk::Format::eD32Sfloat;
  depthAttachment.samples = vk::SampleCountFlagBits::e1;
  depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
  depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
  depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
  depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
  depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

  std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment,
                                                          depthAttachment};

  vk::AttachmentReference colorRef{0, vk::ImageLayout::eColorAttachmentOptimal};
  vk::AttachmentReference depthRef{
      1, vk::ImageLayout::eDepthStencilAttachmentOptimal};
  vk::SubpassDescription subpass{};
  subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorRef;
  subpass.pDepthStencilAttachment = &depthRef;

  vk::SubpassDependency dep{};
  dep.srcSubpass = VK_SUBPASS_EXTERNAL;
  dep.dstSubpass = 0;
  dep.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
                     vk::PipelineStageFlagBits::eEarlyFragmentTests;
  dep.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
                     vk::PipelineStageFlagBits::eEarlyFragmentTests;
  dep.srcAccessMask = {};
  dep.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite |
                      vk::AccessFlagBits::eDepthStencilAttachmentWrite;

  vk::RenderPassCreateInfo info{};
  info.attachmentCount = static_cast<uint32_t>(attachments.size());
  info.pAttachments = attachments.data();
  info.subpassCount = 1;
  info.pSubpasses = &subpass;
  info.dependencyCount = 1;
  info.pDependencies = &dep;
  renderPass = vk::raii::RenderPass(device, info);
}
void App::createGraphicsPipeline()
{
  auto shaderCode = readFile("../shaders/slang.spv");
  vk::raii::ShaderModule shaderModule = createShaderModule(shaderCode);
  vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
  vertShaderStageInfo.module = shaderModule;
  vertShaderStageInfo.pName = "vertMain";
  vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
  fragShaderStageInfo.module = shaderModule;
  fragShaderStageInfo.pName = "fragMain";
  vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                      fragShaderStageInfo};
  auto bindingDescription = Vertex::getBindingDescription();
  auto attributeDescriptions = Vertex::getAttributeDescriptions();
  vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputInfo.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
  vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
  vk::PipelineViewportStateCreateInfo viewportState{};
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  vk::PipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.depthClampEnable = vk::False;
  rasterizer.rasterizerDiscardEnable = vk::False;
  rasterizer.polygonMode = vk::PolygonMode::eFill;
  rasterizer.cullMode = vk::CullModeFlagBits::eBack;
  rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
  rasterizer.depthBiasEnable = vk::False;
  rasterizer.lineWidth = 1.0f;

  vk::PipelineMultisampleStateCreateInfo multisampling{};

  multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
  multisampling.sampleShadingEnable = vk::False;
  vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.blendEnable = vk::False;
  colorBlendAttachment.colorWriteMask =
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
  vk::PipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.logicOpEnable = vk::False;
  colorBlending.logicOp = vk::LogicOp::eCopy;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport,
                                                 vk::DynamicState::eScissor};
  vk::PipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicState.pDynamicStates = dynamicStates.data();
  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
  vk::DescriptorSetLayout setLayouts[] = {*descriptorSetLayout};
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = setLayouts;
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);
  vk::PipelineDepthStencilStateCreateInfo depthStencil{};
  depthStencil.depthTestEnable = vk::True;
  depthStencil.depthWriteEnable = vk::True;
  depthStencil.depthCompareOp = vk::CompareOp::eLess;
  depthStencil.depthBoundsTestEnable = vk::False;
  depthStencil.stencilTestEnable = vk::False;

  vk::GraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo{};
  vkGraphicsPipelineCreateInfo.stageCount = 2;
  vkGraphicsPipelineCreateInfo.pStages = shaderStages;
  vkGraphicsPipelineCreateInfo.pVertexInputState = &vertexInputInfo;
  vkGraphicsPipelineCreateInfo.pInputAssemblyState = &inputAssembly;
  vkGraphicsPipelineCreateInfo.pViewportState = &viewportState;
  vkGraphicsPipelineCreateInfo.pRasterizationState = &rasterizer;
  vkGraphicsPipelineCreateInfo.pMultisampleState = &multisampling;
  vkGraphicsPipelineCreateInfo.pDepthStencilState = &depthStencil;
  vkGraphicsPipelineCreateInfo.pColorBlendState = &colorBlending;
  vkGraphicsPipelineCreateInfo.pDynamicState = &dynamicState;
  vkGraphicsPipelineCreateInfo.layout = pipelineLayout;
  vkGraphicsPipelineCreateInfo.renderPass = *renderPass;
  vkGraphicsPipelineCreateInfo.subpass = 0;

  graphicsPipeline =
      vk::raii::Pipeline(device, nullptr, vkGraphicsPipelineCreateInfo);
}

vk::raii::ShaderModule
App::createShaderModule(const std::vector<char> &code) const
{
  vk::ShaderModuleCreateInfo createInfo{};
  createInfo.codeSize = code.size() * sizeof(char);
  createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
  vk::raii::ShaderModule shaderModule{device, createInfo};
  return shaderModule;
}

void App::createCommandPool()
{
  vk::CommandPoolCreateInfo poolInfo{};
  poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
  poolInfo.queueFamilyIndex = queueIndex;
  commandPool = vk::raii::CommandPool(device, poolInfo);
}

void App::createCommandBuffer()
{
  vk::CommandBufferAllocateInfo allocInfo{};
  allocInfo.commandPool = commandPool;
  allocInfo.level = vk::CommandBufferLevel::ePrimary;
  allocInfo.commandBufferCount = 1;
  commandBuffer =
      std::move(vk::raii::CommandBuffers(device, allocInfo).front());
}

void App::recordCommandBuffer(uint32_t imageIndex)
{
  commandBuffer.begin({});

  std::array<vk::ClearValue, 2> clearValues{};
  clearValues[0].color = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
  clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
  vk::RenderPassBeginInfo renderPassBeginInfo{};
  renderPassBeginInfo.renderPass = *renderPass;
  renderPassBeginInfo.framebuffer = *swapChainFramebuffers[imageIndex];
  renderPassBeginInfo.renderArea =
      vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent);
  renderPassBeginInfo.clearValueCount =
      static_cast<uint32_t>(clearValues.size());
  renderPassBeginInfo.pClearValues = clearValues.data();

  commandBuffer.beginRenderPass(renderPassBeginInfo,
                                vk::SubpassContents::eInline);
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                             *graphicsPipeline);
  commandBuffer.setViewport(
      0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width),
                      static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
  commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));
  commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                   *pipelineLayout, 0, *descriptorSet, nullptr);
  vk::Buffer vertexBuffers[] = {*vertexBuffer};
  vk::DeviceSize offsets[] = {0};
  commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
  commandBuffer.draw(static_cast<uint32_t>(vertices.size()), 1, 0, 0);
  commandBuffer.endRenderPass();

  commandBuffer.end();
}

void App::createSyncObjects()
{
  presentCompleteSemaphore =
      vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
  renderFinishedSemaphore =
      vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
  vk::FenceCreateInfo vkFenceCreateInfo{};
  vkFenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
  drawFence = vk::raii::Fence(device, vkFenceCreateInfo);
}

void App::createDescriptorSetLayout()
{
  vk::DescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

  vk::DescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.bindingCount = 1;
  layoutInfo.pBindings = &uboLayoutBinding;
  descriptorSetLayout = vk::raii::DescriptorSetLayout(device, layoutInfo);
}

uint32_t App::findMemoryType(uint32_t typeFilter,
                             vk::MemoryPropertyFlags properties) const
{
  auto memProperties = physicalDevice.getMemoryProperties();
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
  {
    if ((typeFilter & (1u << i)) &&
        (memProperties.memoryTypes[i].propertyFlags & properties) ==
            properties)
    {
      return i;
    }
  }
  throw std::runtime_error("failed to find suitable memory type!");
}

void App::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                       vk::MemoryPropertyFlags properties,
                       vk::raii::Buffer &buffer,
                       vk::raii::DeviceMemory &bufferMemory)
{
  vk::BufferCreateInfo bufferInfo{};
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = vk::SharingMode::eExclusive;
  buffer = vk::raii::Buffer(device, bufferInfo);

  vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
  vk::MemoryAllocateInfo allocInfo{};
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memRequirements.memoryTypeBits, properties);
  bufferMemory = vk::raii::DeviceMemory(device, allocInfo);

  buffer.bindMemory(*bufferMemory, 0);
}

void App::createUniformBuffer()
{
  vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
  createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
               vk::MemoryPropertyFlagBits::eHostVisible |
                   vk::MemoryPropertyFlagBits::eHostCoherent,
               uniformBuffer, uniformBufferMemory);
  uniformBufferMapped = uniformBufferMemory.mapMemory(0, bufferSize);
}

void App::createVertexBuffer()
{
  vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
  createBuffer(bufferSize, vk::BufferUsageFlagBits::eVertexBuffer,
               vk::MemoryPropertyFlagBits::eHostVisible |
                   vk::MemoryPropertyFlagBits::eHostCoherent,
               vertexBuffer, vertexBufferMemory);
  void *data = vertexBufferMemory.mapMemory(0, bufferSize);
  std::memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
  vertexBufferMemory.unmapMemory();
}

void App::createDescriptorPool()
{
  vk::DescriptorPoolSize poolSize{};
  poolSize.type = vk::DescriptorType::eUniformBuffer;
  poolSize.descriptorCount = 1;

  vk::DescriptorPoolCreateInfo poolInfo{};
  poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
  poolInfo.poolSizeCount = 1;
  poolInfo.pPoolSizes = &poolSize;
  poolInfo.maxSets = 1;
  descriptorPool = vk::raii::DescriptorPool(device, poolInfo);
}

void App::createDescriptorSet()
{
  vk::DescriptorSetLayout layouts[] = {*descriptorSetLayout};
  vk::DescriptorSetAllocateInfo allocInfo{};
  allocInfo.descriptorPool = *descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = layouts;
  descriptorSet =
      std::move(vk::raii::DescriptorSets(device, allocInfo).front());

  vk::DescriptorBufferInfo bufferInfo{};
  bufferInfo.buffer = *uniformBuffer;
  bufferInfo.offset = 0;
  bufferInfo.range = sizeof(UniformBufferObject);

  vk::WriteDescriptorSet descriptorWrite{};
  descriptorWrite.dstSet = *descriptorSet;
  descriptorWrite.dstBinding = 0;
  descriptorWrite.dstArrayElement = 0;
  descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
  descriptorWrite.descriptorCount = 1;
  descriptorWrite.pBufferInfo = &bufferInfo;
  device.updateDescriptorSets(descriptorWrite, nullptr);
}

void App::updateUniformBuffer()
{
  float t = static_cast<float>(glfwGetTime());
  UniformBufferObject ubo{};

  // ubo.model = glm::mat4(1.0f);
  ubo.model = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

  // ubo.view = glm::mat4(1.0f);
  ubo.view = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                       0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

  // ubo.proj = glm::mat4(1.0f);
  ubo.proj = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                       0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

  ubo.proj[1][1] *= -1;
  ubo.proj[1][3] *= -1;
  std::memcpy(uniformBufferMapped, &ubo, sizeof(ubo));
}
