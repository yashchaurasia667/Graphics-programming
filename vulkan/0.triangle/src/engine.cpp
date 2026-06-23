#include "engine.hpp"
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>

#define VOLK_IMPLEMENTATION
#define VULKAN_NO_HPP_STRUCT_CONSTRUCTORS

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

void showError(const char *err) { std::cerr << err << std::endl; }

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

  return true;
}

void Engine::run() {}

void Engine::shutdown() {}

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
        // prefer queue familes that have graphics and presenet
        if (hasGfxQueue && hasPresentQueue) {
          return dev;
        }
      }
    }
  }
  return nullptr;
}

