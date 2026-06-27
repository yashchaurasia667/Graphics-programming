#define VOLK_IMPLEMENTATION
#define VULKAN_NO_HPP_STRUCT_CONSTRUCTORS
#define VMA_IMPLEMENTATION

#include "engine.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

#include <cstring>
#include <fstream>
#include <sstream>

void showError(const char *err) { std::cerr << err << std::endl; }
std::string readTextFile(const std::string &filepath) {
  std::ifstream infile(filepath);
  if (infile.is_open()) {
    std::stringstream buffer;
    buffer << infile.rdbuf();
    const std::string output = buffer.str();
    infile.close();
    return output;
  }
  return std::string();
}

bool Engine::initialize() {
  SDL_InitSubSystem(SDL_INIT_VIDEO);
  window = SDL_CreateWindow("Triangle app", WIDTH, HEIGHT, SDL_WINDOW_VULKAN);
  if (window == nullptr) {
    showError("Failed to create an sdl window");
    return false;
  }
  std::cout << "Initialized SDL window" << std::endl;

  if (!initVulkan()) {
    showError("Failed to initialize vulkan");
    return false;
  }

  return true;
}

void Engine::run() {
  running = true;
  while (running) {
    SDL_Event event{0};
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      }
    }
    render();
  }
}

void Engine::shutdown() { vkDeviceWaitIdle(device); }

bool Engine::initVulkan() {
  if (instance = createVulkanInstance(); !instance) {
    return false;
  }
  std::cout << "vk instance created" << std::endl;
  if (surface = createSurface(); !surface) {
    return false;
  }
  std::cout << "vk surface created" << std::endl;
  if (physicalDevice = pickPhysicalDevice(); !physicalDevice) {
    return false;
  }
  std::cout << "picked a physical device" << std::endl;
  if (!createDevice(physicalDevice)) {
    return false;
  }
  std::cout << "created a logical device" << std::endl;
  if (!initializeVMA()) {
    return false;
  }
  std::cout << "initialized vma" << std::endl;
  if (swapchain = createSwapchain(WIDTH, HEIGHT); !swapchain) {
    return false;
  }
  std::cout << "created swapchain" << std::endl;
  if (!createShaders()) {
    return false;
  }
  std::cout << "Created vertex and fragment shaders" << std::endl;
  if (pipeline = createGraphicsPipeline(); !pipeline.handle) {
    return false;
  }
  std::cout << "Created graphics pipeline" << std::endl;
  if (!createSyncResources()) {
    return false;
  }
  std::cout << "created sync resources" << std::endl;
  if (!createCommandBuffers()) {
    return false;
  }
  std::cout << "Command buffer created" << std::endl;

  return true;
}

// UTILITY FUNCTIONS
VkInstance Engine::createVulkanInstance() {
  if (volkInitialize() != VK_SUCCESS) {
    showError("Failed to initialize volk");
    return nullptr;
  }
  VULKAN_HPP_DEFAULT_DISPATCHER.init();

  VkApplicationInfo appInfo{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "Triangle app",
      .apiVersion = VK_API_VERSION_1_3,
  };

  // SDL EXTENSIONS
  uint32_t sdlExtCount{0};
  const char *const *sdlExtensions =
      SDL_Vulkan_GetInstanceExtensions(&sdlExtCount);
  auto availableExtensions = vk::enumerateInstanceExtensionProperties();

  for (uint32_t i = 0; i < sdlExtCount; i++) {
    bool found = std::ranges::any_of(
        availableExtensions,
        [ext = sdlExtensions[i]](auto const &availableExtension) {
          return strcmp(availableExtension.extensionName, ext) == 0;
        });
    if (!found) {
      showError("Missing required SDL extensions ");
      return nullptr;
    }
  }
  // VALIDATION LAYERS
  std::vector<const char *> requestedLayers = {"VK_LAYER_KHRONOS_validation"};
  auto layerProperties = vk::enumerateInstanceLayerProperties();
  auto unsupportedIt = std::ranges::find_if(
      requestedLayers, [&layerProperties](auto const &requestedLayer) {
        return std::ranges::none_of(
            layerProperties, [requestedLayer](auto const &layerProperty) {
              return strcmp(layerProperty.layerName, requestedLayer);
            });
      });
  if (unsupportedIt != requestedLayers.end()) {
    showError("Required Layer not supported");
    return nullptr;
  }

  VkInstanceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = static_cast<uint32_t>(requestedLayers.size()),
      .ppEnabledLayerNames = requestedLayers.data(),
      .enabledExtensionCount = sdlExtCount,
      .ppEnabledExtensionNames = sdlExtensions,
  };

  VkInstance instance = nullptr;
  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
    showError("Failed to create vk instance");
    return nullptr;
  }
  volkLoadInstance(instance);

  return instance;
}

VkSurfaceKHR Engine::createSurface() const {
  VkSurfaceKHR surface = nullptr;
  SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface);
  return surface;
}

VkPhysicalDevice Engine::pickPhysicalDevice() {
  uint32_t devCount = 0;
  vkEnumeratePhysicalDevices(instance, &devCount, nullptr);
  if (devCount == 0) {
    showError("No vulkan supported devices found");
    return nullptr;
  }

  std::vector<VkPhysicalDevice> devices(devCount);
  vkEnumeratePhysicalDevices(instance, &devCount, devices.data());

  VkPhysicalDevice physicalDev = devices[0];

  for (auto &dev : devices) {

    uint32_t extCount = 0;
    vkEnumerateDeviceExtensionProperties(dev, nullptr, &extCount, nullptr);
    std::vector<VkExtensionProperties> devProps(extCount);
    vkEnumerateDeviceExtensionProperties(dev, nullptr, &extCount,
                                         devProps.data());

    bool hasSwapchainSupport =
        std::ranges::any_of(devProps, [](auto const &ext) {
          return strcmp(ext.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) ==
                 0;
        });

    if (hasSwapchainSupport) {
      bool hasGfxQueue = false;
      bool hasPresentQueue = false;

      uint32_t queueFamCount = 0;
      vkGetPhysicalDeviceQueueFamilyProperties2(dev, &queueFamCount, nullptr);
      std::vector<VkQueueFamilyProperties2> queueFamProps(
          queueFamCount,
          {.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2});
      vkGetPhysicalDeviceQueueFamilyProperties2(dev, &queueFamCount,
                                                queueFamProps.data());

      for (size_t currentFamIdx = 0; currentFamIdx < queueFamProps.size();
           currentFamIdx++) {
        const auto &props = queueFamProps[currentFamIdx];
        if (props.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
          gfxQueueFamIdx = currentFamIdx;
          hasGfxQueue = true;
        }

        // ensure it has presentation support
        VkBool32 hasPresentSupport = false;
        if (vkGetPhysicalDeviceSurfaceSupportKHR(dev, currentFamIdx, surface,
                                                 &hasPresentSupport) ==
            VK_SUCCESS) {
          if (hasPresentSupport) {
            presentQueueFamIdx = currentFamIdx;
            hasPresentQueue = true;
          }
        }
        // prefer queue families that have graphics and present
        if (hasGfxQueue && hasPresentQueue) {
          VkPhysicalDeviceProperties devProperties;
          vkGetPhysicalDeviceProperties(dev, &devProperties);

          if (devProperties.deviceType ==
              VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            std::cout << "Selected discrete GPU: " << devProperties.deviceName
                      << std::endl;
            return dev;
          }
        }
      }
    }
  }
  return physicalDev;
}

bool Engine::createDevice(VkPhysicalDevice physicalDevice) {
  float queuePriority = 1.0f;

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfo{VkDeviceQueueCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = gfxQueueFamIdx,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority}};

  if (gfxQueueFamIdx != presentQueueFamIdx) {
    queueCreateInfo.push_back(VkDeviceQueueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = presentQueueFamIdx,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority});
  }

  VkPhysicalDeviceVulkan13Features supportedFeatures13{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
      .pNext = nullptr};
  VkPhysicalDeviceVulkan12Features supportedFeatures12{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
      .pNext = &supportedFeatures13};
  VkPhysicalDeviceFeatures2 supportedFeatures10{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
      .pNext = &supportedFeatures12};
  vkGetPhysicalDeviceFeatures2(physicalDevice, &supportedFeatures10);

  if (!supportedFeatures13.dynamicRendering ||
      !supportedFeatures13.synchronization2 ||
      !supportedFeatures12.timelineSemaphore) {
    showError("Physical devices does not meet the feature requirement");
    return false;
  }

  VkPhysicalDeviceVulkan13Features features13{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
      .pNext = nullptr,
      .synchronization2 = VK_TRUE, // <-- was missing
      .dynamicRendering = VK_TRUE};
  VkPhysicalDeviceVulkan12Features features12{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
      .pNext = &features13,
      .timelineSemaphore = VK_TRUE};
  VkPhysicalDeviceFeatures2 features10{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
      .pNext = &features12};

  const std::vector<const char *> deviceExtensions{
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  VkDeviceCreateInfo devCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = &features10,
      .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfo.size()),
      .pQueueCreateInfos = queueCreateInfo.data(),
      .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
      .ppEnabledExtensionNames = deviceExtensions.data(),
      .pEnabledFeatures = nullptr};

  if (vkCreateDevice(physicalDevice, &devCreateInfo, nullptr, &device) !=
      VK_SUCCESS) {
    return false;
  }

  vkGetDeviceQueue(device, gfxQueueFamIdx, 0, &gfxQueue);
  if (!gfxQueue) {
    showError("Couldn't get the graphics queue");
    return false;
  }
  presentQueue = gfxQueue;

  if (gfxQueueFamIdx != presentQueueFamIdx) {
    vkGetDeviceQueue(device, presentQueueFamIdx, 0, &presentQueue);
    if (!presentQueue) {
      showError("Couldn't get the present queue");
      return false;
    }
  }

  return true;
}

bool Engine::initializeVMA() {
  VmaVulkanFunctions vmaFuncInfo{};
  VmaAllocatorCreateInfo vmaAllocInfo{
      .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
      .physicalDevice = physicalDevice,
      .device = device,
      .pVulkanFunctions = &vmaFuncInfo,
      .instance = instance,
      .vulkanApiVersion = vk::ApiVersion13};

  vmaImportVulkanFunctionsFromVolk(&vmaAllocInfo, &vmaFuncInfo);
  if (vmaCreateAllocator(&vmaAllocInfo, &vmaAllocator) != VK_SUCCESS) {
    return false;
  }
  return true;
}

VkSwapchainKHR Engine::createSwapchain(uint32_t width, uint32_t height) {
  VkSurfaceCapabilitiesKHR surfaceCaps{};
  if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
                                                &surfaceCaps) != VK_SUCCESS) {
    showError("Couldn't get the surface capabilities");
    return nullptr;
  }

  VkSwapchainCreateInfoKHR swapchainCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = surface,
      .minImageCount = surfaceCaps.minImageCount,
      .imageFormat = swapchainFormat,
      .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
      .imageExtent{.width = width, .height = height},
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = VK_PRESENT_MODE_FIFO_KHR};

  VkSwapchainKHR swapchainLocal = nullptr;
  if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr,
                           &swapchainLocal) != VK_SUCCESS) {
    showError("Failed to create swapchain");
    return nullptr;
  }

  uint32_t imageCount = 0;
  vkGetSwapchainImagesKHR(device, swapchainLocal, &imageCount, nullptr);
  swapchainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(device, swapchainLocal, &imageCount,
                          swapchainImages.data());
  swapchainImageViews.resize(imageCount);

  for (uint32_t i = 0; i < imageCount; i++) {
    VkImageViewCreateInfo imgViewInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = swapchainImages[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = swapchainFormat,
        .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                          .levelCount = 1,
                          .layerCount = 1}};

    if (vkCreateImageView(device, &imgViewInfo, nullptr,
                          &swapchainImageViews[i]) != VK_SUCCESS) {
      showError("Error creating swapchain image view");
      return nullptr;
    }
  }

  VkImageCreateInfo depthCreateInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = depthFormat,
      .extent{.width = width, .height = height, .depth = 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

  VmaAllocationCreateInfo allocInfo{
      .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
      .usage = VMA_MEMORY_USAGE_AUTO};
  if (vmaCreateImage(vmaAllocator, &depthCreateInfo, &allocInfo, &depthImage,
                     &depthImageAllocation, nullptr) != VK_SUCCESS) {
    showError("Error allocating depth image");
    return nullptr;
  }

  VkImageViewCreateInfo depthImageViewInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = depthImage,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = depthFormat,
      .subresourceRange{
          .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
          .levelCount = 1,
          .layerCount = 1,
      }};
  if (vkCreateImageView(device, &depthImageViewInfo, nullptr,
                        &depthImageView) != VK_SUCCESS) {
    showError("Error creating depth image view");
    return nullptr;
  }

  return swapchainLocal;
}

VkShaderModule Engine::createShaderModule(const std::string &filename,
                                          shaderc_shader_kind kind) const {
  const std::string shaderSrc = "shaders/" + filename;
  const std::string src = readTextFile(shaderSrc);
  if (src.empty()) {
    std::cerr << "Failed to read file: " << shaderSrc << std::endl;
    return nullptr;
  }

  shaderc::Compiler compiler;
  shaderc::CompileOptions opts;
  opts.SetTargetEnvironment(shaderc_target_env_vulkan,
                            shaderc_env_version_vulkan_1_3);
  opts.SetTargetSpirv(shaderc_spirv_version_1_6);
  opts.SetOptimizationLevel(shaderc_optimization_level_performance);
  shaderc::CompilationResult result =
      compiler.CompileGlslToSpv(src, kind, filename.c_str(), opts);

  if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
    std::cerr << "Shader compilation error: " << result.GetErrorMessage()
              << std::endl;
    return nullptr;
  }

  std::vector<uint32_t> spv = {result.cbegin(), result.cend()};
  VkShaderModuleCreateInfo moduleCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = spv.size() * sizeof(uint32_t),
      .pCode = spv.data()};
  VkShaderModule module = nullptr;
  if (vkCreateShaderModule(device, &moduleCreateInfo, nullptr, &module) !=
      VK_SUCCESS) {
    showError("Failed to create shader module");
    return nullptr;
  }
  return module;
}

bool Engine::createShaders() {
  if (vertexShader = createShaderModule("shader.vert", shaderc_vertex_shader);
      !vertexShader) {
    showError("Failed to create vertex shader");
    return false;
  }
  if (fragmentShader =
          createShaderModule("shader.frag", shaderc_fragment_shader);
      !fragmentShader) {
    showError("Failed to create fragment shader");
    return false;
  }
  return true;
}

Pipeline Engine::createGraphicsPipeline() const {
  const char *entryPoint = "main";
  std::vector<VkPipelineShaderStageCreateInfo> shaderStage{
      {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
       .stage = VK_SHADER_STAGE_VERTEX_BIT,
       .module = vertexShader,
       .pName = entryPoint},
      {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
       .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
       .module = fragmentShader,
       .pName = entryPoint}};

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE};

  VkPipelineDepthStencilStateCreateInfo depthStencilInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = VK_TRUE,
      .depthWriteEnable = VK_TRUE,
      .depthCompareOp = VK_COMPARE_OP_LESS,
      .stencilTestEnable = VK_FALSE};
  VkPipelineViewportStateCreateInfo viewportInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .pViewports = nullptr,
      .scissorCount = 1,
      .pScissors = nullptr};

  VkPipelineRasterizationStateCreateInfo rasterInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_CLOCKWISE,
      .lineWidth = 1.0f};

  VkPipelineMultisampleStateCreateInfo multiSampleInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT};

  VkPipelineColorBlendAttachmentState attachState{
      .blendEnable = VK_FALSE,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};
  VkPipelineColorBlendStateCreateInfo blendInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments = &attachState};

  std::vector<VkDynamicState> dynamicState{VK_DYNAMIC_STATE_VIEWPORT,
                                           VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicStateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = static_cast<uint32_t>(dynamicState.size()),
      .pDynamicStates = dynamicState.data()};
  VkPipelineRenderingCreateInfo renderInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &swapchainFormat,
      .depthAttachmentFormat = depthFormat};

  Pipeline pipeline;
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 0,
      .pushConstantRangeCount = 0};
  if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                             &pipeline.layout) != VK_SUCCESS) {
    showError("Failed to create graphics pipeline layout");
    return Pipeline{};
  }

  VkGraphicsPipelineCreateInfo pipelineInfo{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = &renderInfo,
      .stageCount = static_cast<uint32_t>(shaderStage.size()),
      .pStages = shaderStage.data(),
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssemblyInfo,
      .pViewportState = &viewportInfo,
      .pRasterizationState = &rasterInfo,
      .pMultisampleState = &multiSampleInfo,
      .pDepthStencilState = &depthStencilInfo,
      .pColorBlendState = &blendInfo,
      .pDynamicState = &dynamicStateInfo,
      .layout = pipeline.layout,
      .renderPass = VK_NULL_HANDLE};
  if (vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineInfo, nullptr,
                                &pipeline.handle) != VK_SUCCESS) {
    showError("Failed to create a pipeline");
    return Pipeline{};
  }
  return pipeline;
}

bool Engine::createSyncResources() {
  // FIX 1: create imageAcquireSemaphores (was missing entirely)
  imageAcquireSemaphores.resize(MaxFramesInFlight);
  for (VkSemaphore &sem : imageAcquireSemaphores) {
    VkSemaphoreCreateInfo semInfo{.sType =
                                      VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    if (vkCreateSemaphore(device, &semInfo, nullptr, &sem) != VK_SUCCESS) {
      showError("Failed to create image acquire semaphore");
      return false;
    }
  }

  VkSemaphoreTypeCreateInfo semaphoreTypeInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
      .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
      .initialValue = timelineValue};
  VkSemaphoreCreateInfo semaphoreInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = &semaphoreTypeInfo};
  if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &timelineSemaphore) !=
      VK_SUCCESS) {
    showError("Failed to create timeline semaphore");
    return false;
  }

  for (FrameResources &res : frameResources) {
    VkSemaphoreCreateInfo binarySemInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    if (vkCreateSemaphore(device, &binarySemInfo, nullptr,
                          &res.workCompleteSemaphore) != VK_SUCCESS) {
      showError("Error creating per frame render complete semaphore");
      return false;
    }
  }

  return true;
}

bool Engine::createCommandBuffers() {
  for (FrameResources &res : frameResources) {
    VkCommandPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = gfxQueueFamIdx};
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &res.commandPool) !=
        VK_SUCCESS) {
      showError("Unable to create command buffer pool");
      return false;
    }

    VkCommandBufferAllocateInfo cmdAllocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = res.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1};

    // FIX 2: actually allocate the command buffer (was missing the call)
    if (vkAllocateCommandBuffers(device, &cmdAllocInfo, &res.commandBuffer) !=
        VK_SUCCESS) {
      showError("Failed to allocate command buffer");
      return false;
    }
  }
  return true;
}

void Engine::render() {
  uint64_t frameId = ++timelineValue;

  // FIX 3: guard against uint64_t underflow on the first MaxFramesInFlight
  // frames
  uint64_t waitForId = (frameId > static_cast<uint64_t>(MaxFramesInFlight))
                           ? frameId - static_cast<uint64_t>(MaxFramesInFlight)
                           : 0;

  VkSemaphore imageAcquireSemaphore =
      imageAcquireSemaphores[frameId % imageAcquireSemaphores.size()];
  size_t frameResourceIndex = frameId % MaxFramesInFlight;
  FrameResources &res = frameResources[frameResourceIndex];

  uint32_t imageIndex = 0;
  VkResult acquireResult =
      vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
                            imageAcquireSemaphore, VK_NULL_HANDLE, &imageIndex);

  if (acquireResult == VK_SUCCESS) {
    VkSemaphoreWaitInfo waitInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
                                 .semaphoreCount = 1,
                                 .pSemaphores = &timelineSemaphore,
                                 .pValues = &waitForId};
    vkWaitSemaphores(device, &waitInfo, UINT64_MAX);
    vkResetCommandPool(device, res.commandPool, 0);

    VkCommandBufferBeginInfo cmdBeginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
    vkBeginCommandBuffer(res.commandBuffer, &cmdBeginInfo);

    std::vector<VkImageMemoryBarrier2> layoutBarriers{
        {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
         .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
         .srcAccessMask = 0,
         .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
         .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
         .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
         .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         .image = swapchainImages[imageIndex],
         .subresourceRange{
             .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
             .baseMipLevel = 0,
             .levelCount = 1,
             .baseArrayLayer = 0,
             .layerCount = 1,
         }},
        {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
         .srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
         .srcAccessMask = 0,
         .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                         VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
         .dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
         .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
         .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
         .image = depthImage,
         .subresourceRange{
             .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
             .baseMipLevel = 0,
             .levelCount = 1,
             .baseArrayLayer = 0,
             .layerCount = 1,
         }}};
    VkDependencyInfo depInfo{.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                             .imageMemoryBarrierCount =
                                 static_cast<uint32_t>(layoutBarriers.size()),
                             .pImageMemoryBarriers = layoutBarriers.data()};
    vkCmdPipelineBarrier2(res.commandBuffer, &depInfo);

    VkRenderingAttachmentInfo colorAttachInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = swapchainImageViews[imageIndex],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue{.color{0, 0, 1, 1}}};
    VkRenderingAttachmentInfo depthAttachInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = depthImageView,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue{.depthStencil{1.0f, 0}}};
    VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea{.offset{.x = 0, .y = 0},
                    .extent{.width = WIDTH, .height = HEIGHT}},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachInfo,
        .pDepthAttachment = &depthAttachInfo};
    vkCmdBeginRendering(res.commandBuffer, &renderingInfo);
    vkCmdBindPipeline(res.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline.handle);

    VkViewport viewport{.x = 0,
                        .y = 0,
                        .width = static_cast<float>(WIDTH),
                        .height = static_cast<float>(HEIGHT)};
    vkCmdSetViewport(res.commandBuffer, 0, 1, &viewport);
    VkRect2D scissor{.offset{.x = 0, .y = 0},
                     .extent{.width = WIDTH, .height = HEIGHT}};
    vkCmdSetScissor(res.commandBuffer, 0, 1, &scissor);

    vkCmdDraw(res.commandBuffer, 3, 1, 0, 0);
    vkCmdEndRendering(res.commandBuffer);

    VkImageMemoryBarrier2 presentLayoutBarrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_NONE,
        .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .image = swapchainImages[imageIndex],
        .subresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }};
    VkDependencyInfo presentDepInfo{.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                                    .imageMemoryBarrierCount = 1,
                                    .pImageMemoryBarriers =
                                        &presentLayoutBarrier};
    vkCmdPipelineBarrier2(res.commandBuffer, &presentDepInfo);

    vkEndCommandBuffer(res.commandBuffer);

    VkSemaphoreSubmitInfo imageAcquireWaitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = imageAcquireSemaphore,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT};
    std::vector<VkSemaphoreSubmitInfo> semaphoreSignals{
        {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
         .semaphore = res.workCompleteSemaphore,
         .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT},
        {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
         .semaphore = timelineSemaphore,
         .value = frameId,
         .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT}};
    VkCommandBufferSubmitInfo cmdSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = res.commandBuffer,
    };
    VkSubmitInfo2 submitInfo{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                             .waitSemaphoreInfoCount = 1,
                             .pWaitSemaphoreInfos = &imageAcquireWaitInfo,
                             .commandBufferInfoCount = 1,
                             .pCommandBufferInfos = &cmdSubmitInfo,
                             .signalSemaphoreInfoCount =
                                 static_cast<uint32_t>(semaphoreSignals.size()),
                             .pSignalSemaphoreInfos = semaphoreSignals.data()};
    vkQueueSubmit2(gfxQueue, 1, &submitInfo, VK_NULL_HANDLE);

    VkPresentInfoKHR presentInfo{.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                 .waitSemaphoreCount = 1,
                                 .pWaitSemaphores = &res.workCompleteSemaphore,
                                 .swapchainCount = 1,
                                 .pSwapchains = &swapchain,
                                 .pImageIndices = &imageIndex,
                                 .pResults = nullptr};
    vkQueuePresentKHR(gfxQueue, &presentInfo);
  }
}
