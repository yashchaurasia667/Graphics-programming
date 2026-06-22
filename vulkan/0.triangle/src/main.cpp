#define VOLK_IMPLEMENTATION
#include <volk.h>

#define VULKAN_NO_HPP_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <vector>

constexpr uint32_t WIDTH = 1280, HEIGHT = 720;

class TriangleApp {
public:
  void run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
  }

private:
  SDL_Window *window;
  VkInstance instance;
  VkPhysicalDevice physicalDevice = nullptr;
  VkDevice device = nullptr;

  VkQueue queue{VK_NULL_HANDLE};
  uint32_t gfxQueueFamIdx = UINT32_MAX;

  void initWindow() {
    if (SDL_Init(SDL_INIT_VIDEO)) {
      window = SDL_CreateWindow("Vulkan", WIDTH, HEIGHT,
                                SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
      if (!window) {
        std::cerr << "Failed to initialize window" << std::endl;
        throw std::exception();
      }
    }
  }

  void initVulkan() {
    if (!createInstance()) {
      std::cerr << "Failed to create a vulkan instance" << std::endl;
      return;
    }
    // if (!pickPhysicalDevices()) {
    //   std::cerr << "Failed to find a vulkan capable gpu in the system"
    //             << std::endl;
    //   return;
    // }
    // findGraphicsQueue();
    // if (!createDevice()) {
    //   std::cerr << "Failed to create a device" << std::endl;
    //   return;
    // }
  }
  void mainLoop() {}
  void cleanup() {}

  // HELPER FUNCTIONS
  bool createInstance() {
    if (volkInitialize() != VK_SUCCESS) {
      std::cerr << "Error initializing volk" << std::endl;
      return false;
    }

    VkApplicationInfo appInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Triangle app",
        .apiVersion = vk::ApiVersion13,
    };

    // validation layers
    std::vector<const char *> requestedLayers = {"VK_LAYER_KHRONOS_validation"};
    auto layerProperties = vk::enumerateInstanceLayerProperties();
    auto unsupportedLayerIt = std::ranges::find_if(
        requestedLayers, [&layerProperties](auto const &requiredLayer) {
          return std::ranges::none_of(
              layerProperties, [requiredLayer](auto const &layerProperty) {
                return strcmp(layerProperty.layerName, requiredLayer) == 0;
              });
        });
    if (unsupportedLayerIt != requestedLayers.end()) {
      std::cerr << ("Required layer not supported: " +
                    std::string(*unsupportedLayerIt))
                << std::endl;
      return false;
    }

    // SDL extensions
    uint32_t instExtCount = 0;
    const char *const *rawExtensions =
        SDL_Vulkan_GetInstanceExtensions(&instExtCount);
    std::span<const char *const> extensions(rawExtensions, instExtCount);

    auto extensionProperties = vk::enumerateInstanceExtensionProperties();

    auto unsupportedPropertyIt = std::ranges::find_if(
        extensions, [&extensionProperties](const char *requiredExtension) {
          return std::ranges::none_of(
              extensionProperties,
              [requiredExtension](auto const &extensionProperty) {
                return strcmp(extensionProperty.extensionName,
                              requiredExtension) == 0;
              });
        });

    if (unsupportedPropertyIt != extensions.end()) {
      std::cerr << ("Required extension not supported: " +
                               std::string(*unsupportedPropertyIt)) << std::endl;
      return false;
    }

    VkInstanceCreateInfo ci{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = (uint32_t)requestedLayers.size(),
        .ppEnabledLayerNames = requestedLayers.data(),
        .enabledExtensionCount = instExtCount,
        .ppEnabledExtensionNames = extensions.data(),
    };
    vkCreateInstance(&ci, nullptr, &instance);
    volkLoadInstance(instance);

    return true;
  }

  bool pickPhysicalDevices() {
    uint32_t deviceCount{0};
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
      return false;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (auto &dev : devices) {
      VkPhysicalDeviceProperties props{};
      vkGetPhysicalDeviceProperties(dev, &props);
      if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        physicalDevice = dev;
        return true;
      }
    }
    physicalDevice = devices[0];
    return true;
  }

  void findGraphicsQueue() {
    uint32_t queueFamilyCount{0};
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                             nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                             queueFamilies.data());
    for (size_t i = 0; i < queueFamilies.size(); i++) {
      if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        gfxQueueFamIdx = i;
        break;
      }
    }
    SDL_Vulkan_GetPresentationSupport(instance, physicalDevice, gfxQueueFamIdx);
  }

  bool createDevice() {
    const float qfPriorities{0};
    VkDeviceQueueCreateInfo queueCi{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = gfxQueueFamIdx,
        .queueCount = 1,
        .pQueuePriorities = &qfPriorities};

    const std::vector<const char *> deviceExtensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkPhysicalDeviceVulkan12Features enabledVulkan12Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .descriptorIndexing = true,
        .shaderSampledImageArrayNonUniformIndexing = true,
        .descriptorBindingVariableDescriptorCount = true,
        .runtimeDescriptorArray = true,
        .bufferDeviceAddress = true};
    VkPhysicalDeviceVulkan13Features enabledVulkan13Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &enabledVulkan12Features,
        .synchronization2 = true,
        .dynamicRendering = true};
    VkPhysicalDeviceFeatures enabledVulkan10Features{
        .samplerAnisotropy = true,
    };

    VkDeviceCreateInfo deviceCi{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &enabledVulkan13Features,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCi,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &enabledVulkan10Features,
    };
    vkCreateDevice(physicalDevice, &deviceCi, nullptr, &device);
    vkGetDeviceQueue(device, gfxQueueFamIdx, 0, &queue);

    if (!queue) {
      std::cerr << "Couldn't get the graphics queue" << std::endl;
      return false;
    }
    return true;
  }
};

int main() {
  TriangleApp app;
  try {
    app.run();
  } catch (std::exception &e) {
    std::cerr << "EXCEPTION-> " << e.what() << std::endl;
  }
  return EXIT_SUCCESS;
}
