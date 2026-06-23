#pragma once
#include <volk/volk.h>
#include <vulkan/vulkan.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>

constexpr uint32_t WIDTH = 1280, HEIGHT = 720;

class Engine {
public:
  bool initialize();
  void run();
  void shutdown();

private:
  SDL_Window *window = nullptr;
  VkInstance instance = nullptr;
  VkSurfaceKHR surface = nullptr;
  VkPhysicalDevice physicalDevice = nullptr;
  uint32_t gfxQueueFamIdx = UINT32_MAX;
  uint32_t presentQueueFamIdx = UINT32_MAX;

  bool initVulkan();
  VkInstance createVulkanInstance();
  VkSurfaceKHR createSurface() const;
  VkPhysicalDevice pickPhysicalDevice();
};
