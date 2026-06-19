// ============================================================
//  Vulkan Hello Triangle
//  Renders a single interpolated-color triangle using Vulkan
//  and a GLFW window.  No vertex buffers — positions are
//  hard-coded in the vertex shader via gl_VertexIndex.
// ============================================================

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

// ---------------------------------------------------------------------------
//  Config
// ---------------------------------------------------------------------------
static constexpr uint32_t WIDTH = 900;
static constexpr uint32_t HEIGHT = 700;
static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

// Validation layers — disabled in release builds
#ifdef NDEBUG
static constexpr bool ENABLE_VALIDATION = false;
#else
static constexpr bool ENABLE_VALIDATION = true;
#endif

static const std::vector<const char *> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"};

static const std::vector<const char *> DEVICE_EXTENSIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

// ---------------------------------------------------------------------------
//  Helpers
// ---------------------------------------------------------------------------
static std::vector<char> readFile(const std::string &path) {
  std::ifstream f(path, std::ios::ate | std::ios::binary);
  if (!f.is_open())
    throw std::runtime_error("Failed to open file: " + path);
  size_t size = static_cast<size_t>(f.tellg());
  std::vector<char> buf(size);
  f.seekg(0);
  f.read(buf.data(), size);
  return buf;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT /*type*/,
    const VkDebugUtilsMessengerCallbackDataEXT *data, void * /*userdata*/) {
  if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    std::cerr << "[Validation] " << data->pMessage << "\n";
  return VK_FALSE;
}

static VkResult createDebugMessenger(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *ci,
    const VkAllocationCallbacks *alloc, VkDebugUtilsMessengerEXT *out) {
  auto fn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  return fn ? fn(instance, ci, alloc, out) : VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void destroyDebugMessenger(VkInstance instance,
                                  VkDebugUtilsMessengerEXT messenger,
                                  const VkAllocationCallbacks *alloc) {
  auto fn = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  if (fn)
    fn(instance, messenger, alloc);
}

// ---------------------------------------------------------------------------
//  Queue family indices
// ---------------------------------------------------------------------------
struct QueueFamilies {
  std::optional<uint32_t> graphics;
  std::optional<uint32_t> present;
  bool complete() const { return graphics.has_value() && present.has_value(); }
};

// ---------------------------------------------------------------------------
//  Swap-chain support details
// ---------------------------------------------------------------------------
struct SwapchainSupport {
  VkSurfaceCapabilitiesKHR caps;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

// ---------------------------------------------------------------------------
//  Application
// ---------------------------------------------------------------------------
class TriangleApp {
public:
  void run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
  }

private:
  // ── GLFW ────────────────────────────────────────────────────────────────
  GLFWwindow *window_ = nullptr;

  // ── Vulkan core ─────────────────────────────────────────────────────────
  VkInstance instance_ = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT debugMessenger_ = VK_NULL_HANDLE;
  VkSurfaceKHR surface_ = VK_NULL_HANDLE;
  VkPhysicalDevice physDevice_ = VK_NULL_HANDLE;
  VkDevice device_ = VK_NULL_HANDLE;
  VkQueue graphicsQueue_ = VK_NULL_HANDLE;
  VkQueue presentQueue_ = VK_NULL_HANDLE;

  // ── Swapchain ───────────────────────────────────────────────────────────
  VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
  std::vector<VkImage> swapImages_;
  VkFormat swapFormat_ = VK_FORMAT_UNDEFINED;
  VkExtent2D swapExtent_ = {};
  std::vector<VkImageView> swapImageViews_;

  // ── Render pass & pipeline ───────────────────────────────────────────────
  VkRenderPass renderPass_ = VK_NULL_HANDLE;
  VkPipelineLayout pipeLayout_ = VK_NULL_HANDLE;
  VkPipeline pipeline_ = VK_NULL_HANDLE;

  // ── Framebuffers & commands ─────────────────────────────────────────────
  std::vector<VkFramebuffer> framebuffers_;
  VkCommandPool cmdPool_ = VK_NULL_HANDLE;
  std::vector<VkCommandBuffer> cmdBuffers_;

  // ── Sync ─────────────────────────────────────────────────────────────────
  std::vector<VkSemaphore> imageAvailSems_;
  std::vector<VkSemaphore> renderDoneSems_;
  std::vector<VkFence> inFlightFences_;
  uint32_t currentFrame_ = 0;

  bool framebufferResized_ = false;

  // ────────────────────────────────────────────────────────────────────────
  //  GLFW
  // ────────────────────────────────────────────────────────────────────────
  void initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // no OpenGL context

    window_ =
        glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Triangle", nullptr, nullptr);
    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, [](GLFWwindow *w, int, int) {
      auto app = static_cast<TriangleApp *>(glfwGetWindowUserPointer(w));
      app->framebufferResized_ = true;
    });
  }

  // ────────────────────────────────────────────────────────────────────────
  //  Vulkan init
  // ────────────────────────────────────────────────────────────────────────
  void initVulkan() {
    createInstance();
    if (ENABLE_VALIDATION)
      setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createCommandBuffers();
    createSyncObjects();
  }

  // ── Instance ─────────────────────────────────────────────────────────────
  void createInstance() {
    if (ENABLE_VALIDATION && !checkValidationSupport())
      throw std::runtime_error(
          "Validation layers requested but not available.");

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "None";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    auto exts = getRequiredExtensions();

    VkInstanceCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo = &appInfo;
    ci.enabledExtensionCount = static_cast<uint32_t>(exts.size());
    ci.ppEnabledExtensionNames = exts.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCI = makeDebugCI();
    if (ENABLE_VALIDATION) {
      ci.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
      ci.ppEnabledLayerNames = VALIDATION_LAYERS.data();
      ci.pNext = &debugCI;
    }

    if (vkCreateInstance(&ci, nullptr, &instance_) != VK_SUCCESS)
      throw std::runtime_error("Failed to create Vulkan instance.");
  }

  bool checkValidationSupport() {
    uint32_t count;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> layers(count);
    vkEnumerateInstanceLayerProperties(&count, layers.data());

    for (const char *name : VALIDATION_LAYERS) {
      bool found = false;
      for (const auto &l : layers)
        if (strcmp(name, l.layerName) == 0) {
          found = true;
          break;
        }
      if (!found)
        return false;
    }
    return true;
  }

  std::vector<const char *> getRequiredExtensions() {
    uint32_t count;
    const char **glfwExts = glfwGetRequiredInstanceExtensions(&count);
    std::vector<const char *> exts(glfwExts, glfwExts + count);
    if (ENABLE_VALIDATION)
      exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return exts;
  }

  // ── Debug messenger ───────────────────────────────────────────────────────
  VkDebugUtilsMessengerCreateInfoEXT makeDebugCI() {
    VkDebugUtilsMessengerCreateInfoEXT ci{};
    ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    ci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    ci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    ci.pfnUserCallback = debugCallback;
    return ci;
  }

  void setupDebugMessenger() {
    auto ci = makeDebugCI();
    if (createDebugMessenger(instance_, &ci, nullptr, &debugMessenger_) !=
        VK_SUCCESS)
      throw std::runtime_error("Failed to create debug messenger.");
  }

  // ── Surface ───────────────────────────────────────────────────────────────
  void createSurface() {
    if (glfwCreateWindowSurface(instance_, window_, nullptr, &surface_) !=
        VK_SUCCESS)
      throw std::runtime_error("Failed to create window surface.");
  }

  // ── Physical device ───────────────────────────────────────────────────────
  void pickPhysicalDevice() {
    uint32_t count;
    vkEnumeratePhysicalDevices(instance_, &count, nullptr);
    if (count == 0)
      throw std::runtime_error("No Vulkan-capable GPU found.");

    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance_, &count, devices.data());

    for (const auto &d : devices) {
      if (isSuitable(d)) {
        physDevice_ = d;
        break;
      }
    }
    if (physDevice_ == VK_NULL_HANDLE)
      throw std::runtime_error("No suitable GPU found.");

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physDevice_, &props);
    std::cout << "Using GPU: " << props.deviceName << "\n";
  }

  bool isSuitable(VkPhysicalDevice d) {
    return findQueueFamilies(d).complete() && checkDeviceExtensions(d) &&
           swapchainAdequate(d);
  }

  bool checkDeviceExtensions(VkPhysicalDevice d) {
    uint32_t count;
    vkEnumerateDeviceExtensionProperties(d, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> exts(count);
    vkEnumerateDeviceExtensionProperties(d, nullptr, &count, exts.data());

    std::set<std::string> required(DEVICE_EXTENSIONS.begin(),
                                   DEVICE_EXTENSIONS.end());
    for (const auto &e : exts)
      required.erase(e.extensionName);
    return required.empty();
  }

  bool swapchainAdequate(VkPhysicalDevice d) {
    auto sc = querySwapchainSupport(d);
    return !sc.formats.empty() && !sc.presentModes.empty();
  }

  QueueFamilies findQueueFamilies(VkPhysicalDevice d) {
    QueueFamilies qf;
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(d, &count, nullptr);
    std::vector<VkQueueFamilyProperties> props(count);
    vkGetPhysicalDeviceQueueFamilyProperties(d, &count, props.data());

    for (uint32_t i = 0; i < count; ++i) {
      if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        qf.graphics = i;

      VkBool32 presentSupport;
      vkGetPhysicalDeviceSurfaceSupportKHR(d, i, surface_, &presentSupport);
      if (presentSupport)
        qf.present = i;

      if (qf.complete())
        break;
    }
    return qf;
  }

  // ── Logical device ────────────────────────────────────────────────────────
  void createLogicalDevice() {
    QueueFamilies qf = findQueueFamilies(physDevice_);
    std::set<uint32_t> uniqueQueues = {*qf.graphics, *qf.present};

    float priority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueCIs;
    for (uint32_t family : uniqueQueues) {
      VkDeviceQueueCreateInfo qi{};
      qi.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      qi.queueFamilyIndex = family;
      qi.queueCount = 1;
      qi.pQueuePriorities = &priority;
      queueCIs.push_back(qi);
    }

    VkPhysicalDeviceFeatures features{};

    VkDeviceCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    ci.queueCreateInfoCount = static_cast<uint32_t>(queueCIs.size());
    ci.pQueueCreateInfos = queueCIs.data();
    ci.pEnabledFeatures = &features;
    ci.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
    ci.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

    if (ENABLE_VALIDATION) {
      ci.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
      ci.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    }

    if (vkCreateDevice(physDevice_, &ci, nullptr, &device_) != VK_SUCCESS)
      throw std::runtime_error("Failed to create logical device.");

    vkGetDeviceQueue(device_, *qf.graphics, 0, &graphicsQueue_);
    vkGetDeviceQueue(device_, *qf.present, 0, &presentQueue_);
  }

  // ── Swapchain ─────────────────────────────────────────────────────────────
  SwapchainSupport querySwapchainSupport(VkPhysicalDevice d) {
    SwapchainSupport sc;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(d, surface_, &sc.caps);

    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(d, surface_, &count, nullptr);
    sc.formats.resize(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(d, surface_, &count,
                                         sc.formats.data());

    vkGetPhysicalDeviceSurfacePresentModesKHR(d, surface_, &count, nullptr);
    sc.presentModes.resize(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(d, surface_, &count,
                                              sc.presentModes.data());
    return sc;
  }

  VkSurfaceFormatKHR
  chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats) {
    for (const auto &f : formats)
      if (f.format == VK_FORMAT_B8G8R8A8_SRGB &&
          f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        return f;
    return formats[0];
  }

  VkPresentModeKHR
  choosePresentMode(const std::vector<VkPresentModeKHR> &modes) {
    for (const auto &m : modes)
      if (m == VK_PRESENT_MODE_MAILBOX_KHR)
        return m;                    // triple-buffered if available
    return VK_PRESENT_MODE_FIFO_KHR; // vsync fallback
  }

  VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR &caps) {
    if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max())
      return caps.currentExtent;

    int w, h;
    glfwGetFramebufferSize(window_, &w, &h);
    VkExtent2D ext = {static_cast<uint32_t>(w), static_cast<uint32_t>(h)};
    ext.width = std::clamp(ext.width, caps.minImageExtent.width,
                           caps.maxImageExtent.width);
    ext.height = std::clamp(ext.height, caps.minImageExtent.height,
                            caps.maxImageExtent.height);
    return ext;
  }

  void createSwapchain() {
    SwapchainSupport sc = querySwapchainSupport(physDevice_);
    auto format = chooseSurfaceFormat(sc.formats);
    auto mode = choosePresentMode(sc.presentModes);
    auto extent = chooseExtent(sc.caps);

    uint32_t imageCount = sc.caps.minImageCount + 1;
    if (sc.caps.maxImageCount > 0)
      imageCount = std::min(imageCount, sc.caps.maxImageCount);

    VkSwapchainCreateInfoKHR ci{};
    ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    ci.surface = surface_;
    ci.minImageCount = imageCount;
    ci.imageFormat = format.format;
    ci.imageColorSpace = format.colorSpace;
    ci.imageExtent = extent;
    ci.imageArrayLayers = 1;
    ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilies qf = findQueueFamilies(physDevice_);
    uint32_t indices[] = {*qf.graphics, *qf.present};
    if (*qf.graphics != *qf.present) {
      ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      ci.queueFamilyIndexCount = 2;
      ci.pQueueFamilyIndices = indices;
    } else {
      ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    ci.preTransform = sc.caps.currentTransform;
    ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    ci.presentMode = mode;
    ci.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(device_, &ci, nullptr, &swapchain_) != VK_SUCCESS)
      throw std::runtime_error("Failed to create swapchain.");

    vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount, nullptr);
    swapImages_.resize(imageCount);
    vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount,
                            swapImages_.data());

    swapFormat_ = format.format;
    swapExtent_ = extent;
  }

  // ── Image views ───────────────────────────────────────────────────────────
  void createImageViews() {
    swapImageViews_.resize(swapImages_.size());
    for (size_t i = 0; i < swapImages_.size(); ++i) {
      VkImageViewCreateInfo ci{};
      ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      ci.image = swapImages_[i];
      ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
      ci.format = swapFormat_;
      ci.components = {
          VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
          VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
      ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      ci.subresourceRange.baseMipLevel = 0;
      ci.subresourceRange.levelCount = 1;
      ci.subresourceRange.baseArrayLayer = 0;
      ci.subresourceRange.layerCount = 1;

      if (vkCreateImageView(device_, &ci, nullptr, &swapImageViews_[i]) !=
          VK_SUCCESS)
        throw std::runtime_error("Failed to create image view.");
    }
  }

  // ── Render pass ───────────────────────────────────────────────────────────
  void createRenderPass() {
    // Single color attachment — swap-chain image
    VkAttachmentDescription colorAttach{};
    colorAttach.format = swapFormat_;
    colorAttach.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttach.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference ref{};
    ref.attachment = 0;
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &ref;

    // Wait for color output stage before we start writing
    VkSubpassDependency dep{};
    dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    dep.dstSubpass = 0;
    dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.srcAccessMask = 0;
    dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    ci.attachmentCount = 1;
    ci.pAttachments = &colorAttach;
    ci.subpassCount = 1;
    ci.pSubpasses = &subpass;
    ci.dependencyCount = 1;
    ci.pDependencies = &dep;

    if (vkCreateRenderPass(device_, &ci, nullptr, &renderPass_) != VK_SUCCESS)
      throw std::runtime_error("Failed to create render pass.");
  }

  // ── Pipeline ──────────────────────────────────────────────────────────────
  VkShaderModule createShaderModule(const std::vector<char> &code) {
    VkShaderModuleCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.codeSize = code.size();
    ci.pCode = reinterpret_cast<const uint32_t *>(code.data());
    VkShaderModule mod;
    if (vkCreateShaderModule(device_, &ci, nullptr, &mod) != VK_SUCCESS)
      throw std::runtime_error("Failed to create shader module.");
    return mod;
  }

  void createGraphicsPipeline() {
    auto vert = readFile("shaders/triangle.vert.spv");
    auto frag = readFile("shaders/triangle.frag.spv");

    VkShaderModule vertMod = createShaderModule(vert);
    VkShaderModule fragMod = createShaderModule(frag);

    // Shader stages
    VkPipelineShaderStageCreateInfo vertStage{};
    vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStage.module = vertMod;
    vertStage.pName = "main";

    VkPipelineShaderStageCreateInfo fragStage{};
    fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStage.module = fragMod;
    fragStage.pName = "main";

    VkPipelineShaderStageCreateInfo stages[] = {vertStage, fragStage};

    // No vertex buffers — positions are in the shader
    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // Viewport & scissor are set dynamically at draw time
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Standard alpha blending (off — opaque triangle)
    VkPipelineColorBlendAttachmentState blendAttach{};
    blendAttach.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blendAttach.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlend{};
    colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlend.attachmentCount = 1;
    colorBlend.pAttachments = &blendAttach;

    // Dynamic state — viewport & scissor
    VkDynamicState dynStates[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                  VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynState{};
    dynState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynState.dynamicStateCount = 2;
    dynState.pDynamicStates = dynStates;

    // Pipeline layout (no descriptors needed for a bare triangle)
    VkPipelineLayoutCreateInfo layoutCI{};
    layoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    if (vkCreatePipelineLayout(device_, &layoutCI, nullptr, &pipeLayout_) !=
        VK_SUCCESS)
      throw std::runtime_error("Failed to create pipeline layout.");

    VkGraphicsPipelineCreateInfo pipeCI{};
    pipeCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeCI.stageCount = 2;
    pipeCI.pStages = stages;
    pipeCI.pVertexInputState = &vertexInput;
    pipeCI.pInputAssemblyState = &inputAssembly;
    pipeCI.pViewportState = &viewportState;
    pipeCI.pRasterizationState = &rasterizer;
    pipeCI.pMultisampleState = &multisampling;
    pipeCI.pColorBlendState = &colorBlend;
    pipeCI.pDynamicState = &dynState;
    pipeCI.layout = pipeLayout_;
    pipeCI.renderPass = renderPass_;
    pipeCI.subpass = 0;

    if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipeCI, nullptr,
                                  &pipeline_) != VK_SUCCESS)
      throw std::runtime_error("Failed to create graphics pipeline.");

    // Shader modules no longer needed after pipeline is compiled
    vkDestroyShaderModule(device_, vertMod, nullptr);
    vkDestroyShaderModule(device_, fragMod, nullptr);
  }

  // ── Framebuffers ──────────────────────────────────────────────────────────
  void createFramebuffers() {
    framebuffers_.resize(swapImageViews_.size());
    for (size_t i = 0; i < swapImageViews_.size(); ++i) {
      VkFramebufferCreateInfo ci{};
      ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      ci.renderPass = renderPass_;
      ci.attachmentCount = 1;
      ci.pAttachments = &swapImageViews_[i];
      ci.width = swapExtent_.width;
      ci.height = swapExtent_.height;
      ci.layers = 1;

      if (vkCreateFramebuffer(device_, &ci, nullptr, &framebuffers_[i]) !=
          VK_SUCCESS)
        throw std::runtime_error("Failed to create framebuffer.");
    }
  }

  // ── Command pool & buffers ────────────────────────────────────────────────
  void createCommandPool() {
    QueueFamilies qf = findQueueFamilies(physDevice_);
    VkCommandPoolCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ci.queueFamilyIndex = *qf.graphics;
    if (vkCreateCommandPool(device_, &ci, nullptr, &cmdPool_) != VK_SUCCESS)
      throw std::runtime_error("Failed to create command pool.");
  }

  void createCommandBuffers() {
    cmdBuffers_.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo ai{};
    ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    ai.commandPool = cmdPool_;
    ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    ai.commandBufferCount = static_cast<uint32_t>(cmdBuffers_.size());
    if (vkAllocateCommandBuffers(device_, &ai, cmdBuffers_.data()) !=
        VK_SUCCESS)
      throw std::runtime_error("Failed to allocate command buffers.");
  }

  void recordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex) {
    VkCommandBufferBeginInfo bi{};
    bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(cmd, &bi);

    VkClearValue clearColor = {.color = {0.08f, 0.08f, 0.10f, 1.0f}};
    VkRenderPassBeginInfo rpBI{};
    rpBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBI.renderPass = renderPass_;
    rpBI.framebuffer = framebuffers_[imageIndex];
    rpBI.renderArea.offset = {0, 0};
    rpBI.renderArea.extent = swapExtent_;
    rpBI.clearValueCount = 1;
    rpBI.pClearValues = &clearColor;

    vkCmdBeginRenderPass(cmd, &rpBI, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);

    VkViewport vp{};
    vp.x = 0.0f;
    vp.y = 0.0f;
    vp.width = static_cast<float>(swapExtent_.width);
    vp.height = static_cast<float>(swapExtent_.height);
    vp.minDepth = 0.0f;
    vp.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &vp);

    VkRect2D scissor{{0, 0}, swapExtent_};
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    // Draw 3 vertices — no vertex buffer, positions are in the shader
    vkCmdDraw(cmd, 3, 1, 0, 0);

    vkCmdEndRenderPass(cmd);
    if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
      throw std::runtime_error("Failed to record command buffer.");
  }

  // ── Sync objects ──────────────────────────────────────────────────────────
  void createSyncObjects() {
    imageAvailSems_.resize(MAX_FRAMES_IN_FLIGHT);
    renderDoneSems_.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo si{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fi{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fi.flags = VK_FENCE_CREATE_SIGNALED_BIT; // signaled so first wait returns
                                             // immediately

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
      if (vkCreateSemaphore(device_, &si, nullptr, &imageAvailSems_[i]) !=
              VK_SUCCESS ||
          vkCreateSemaphore(device_, &si, nullptr, &renderDoneSems_[i]) !=
              VK_SUCCESS ||
          vkCreateFence(device_, &fi, nullptr, &inFlightFences_[i]) !=
              VK_SUCCESS)
        throw std::runtime_error("Failed to create sync objects.");
    }
  }

  // ────────────────────────────────────────────────────────────────────────
  //  Main loop & draw
  // ────────────────────────────────────────────────────────────────────────
  void mainLoop() {
    while (!glfwWindowShouldClose(window_)) {
      glfwPollEvents();
      drawFrame();
    }
    vkDeviceWaitIdle(device_);
  }

  void drawFrame() {
    // Wait for this frame's fence
    vkWaitForFences(device_, 1, &inFlightFences_[currentFrame_], VK_TRUE,
                    UINT64_MAX);

    // Acquire next swap-chain image
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX,
                                            imageAvailSems_[currentFrame_],
                                            VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      recreateSwapchain();
      return;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
      throw std::runtime_error("Failed to acquire swapchain image.");

    vkResetFences(device_, 1, &inFlightFences_[currentFrame_]);

    // Record
    vkResetCommandBuffer(cmdBuffers_[currentFrame_], 0);
    recordCommandBuffer(cmdBuffers_[currentFrame_], imageIndex);

    // Submit
    VkSemaphore waitSems[] = {imageAvailSems_[currentFrame_]};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSems[] = {renderDoneSems_[currentFrame_]};

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = waitSems;
    submit.pWaitDstStageMask = waitStages;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmdBuffers_[currentFrame_];
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = signalSems;

    if (vkQueueSubmit(graphicsQueue_, 1, &submit,
                      inFlightFences_[currentFrame_]) != VK_SUCCESS)
      throw std::runtime_error("Failed to submit draw command buffer.");

    // Present
    VkPresentInfoKHR present{};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = signalSems;
    present.swapchainCount = 1;
    present.pSwapchains = &swapchain_;
    present.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(presentQueue_, &present);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        framebufferResized_) {
      framebufferResized_ = false;
      recreateSwapchain();
    } else if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to present swapchain image.");
    }

    currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
  }

  // ── Swapchain recreation (window resize) ──────────────────────────────────
  void recreateSwapchain() {
    // Handle minimization — wait until window has non-zero size
    int w = 0, h = 0;
    while (w == 0 || h == 0) {
      glfwGetFramebufferSize(window_, &w, &h);
      glfwWaitEvents();
    }
    vkDeviceWaitIdle(device_);
    cleanupSwapchain();
    createSwapchain();
    createImageViews();
    createFramebuffers();
  }

  // ────────────────────────────────────────────────────────────────────────
  //  Cleanup
  // ────────────────────────────────────────────────────────────────────────
  void cleanupSwapchain() {
    for (auto fb : framebuffers_)
      vkDestroyFramebuffer(device_, fb, nullptr);
    for (auto iv : swapImageViews_)
      vkDestroyImageView(device_, iv, nullptr);
    vkDestroySwapchainKHR(device_, swapchain_, nullptr);
  }

  void cleanup() {
    cleanupSwapchain();

    vkDestroyPipeline(device_, pipeline_, nullptr);
    vkDestroyPipelineLayout(device_, pipeLayout_, nullptr);
    vkDestroyRenderPass(device_, renderPass_, nullptr);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
      vkDestroySemaphore(device_, imageAvailSems_[i], nullptr);
      vkDestroySemaphore(device_, renderDoneSems_[i], nullptr);
      vkDestroyFence(device_, inFlightFences_[i], nullptr);
    }

    vkDestroyCommandPool(device_, cmdPool_, nullptr);
    vkDestroyDevice(device_, nullptr);

    if (ENABLE_VALIDATION)
      destroyDebugMessenger(instance_, debugMessenger_, nullptr);

    vkDestroySurfaceKHR(instance_, surface_, nullptr);
    vkDestroyInstance(instance_, nullptr);

    glfwDestroyWindow(window_);
    glfwTerminate();
  }
};

// ────────────────────────────────────────────────────────────────────────────
int main() {
  try {
    TriangleApp app;
    app.run();
  } catch (const std::exception &e) {
    std::cerr << "Fatal: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
