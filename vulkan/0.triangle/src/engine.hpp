#pragma once

#include <volk/volk.h>
#include <vulkan/vulkan.hpp>

#include <vk_mem_alloc.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <vulkan/vulkan_core.h>

class Engine {
public:
  bool initialize();
  void run();
  void shutdown();

private:
  constexpr static uint32_t WIDTH = 1280, HEIGHT = 720;
  constexpr static uint32_t VulkanVersion{VK_API_VERSION_1_4};
  constexpr static uint32_t MaxFramesInFlight{2};
  constexpr static VkFormat swapchainFormat{VK_FORMAT_B8G8R8A8_SRGB};
  constexpr static VkFormat depthFormat{VK_FORMAT_D32_SFLOAT};

  SDL_Window *window = nullptr;
  VkInstance instance = nullptr;
  VkSurfaceKHR surface = nullptr;
  VmaAllocator vmaAllocator = nullptr;

  VkPhysicalDevice physicalDevice = nullptr;
  VkDevice device = nullptr;

  uint32_t gfxQueueFamIdx = UINT32_MAX;
  uint32_t presentQueueFamIdx = UINT32_MAX;
  VkQueue gfxQueue = nullptr;
  VkQueue presentQueue = nullptr;

  VkSwapchainKHR swapchain = nullptr;
  std::vector<VkImage> swapchainImages;
  std::vector<VkImageView> swapchainImageViews;

  VkImage depthImage = nullptr;
  VmaAllocation depthImageAllocation = nullptr;
  VkImageView depthImageView = nullptr;

  bool initVulkan();
  VkInstance createVulkanInstance();
  VkSurfaceKHR createSurface() const;
  VkPhysicalDevice pickPhysicalDevice();
  bool createDevice(VkPhysicalDevice physicalDevice);
  bool initializeVMA();
  VkSwapchainKHR createSwapchain(uint32_t width, uint32_t height);
};
