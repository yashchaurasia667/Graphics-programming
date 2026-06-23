#include "engine.hpp"
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

int main() {
  Engine app;
  if(app.initialize()) {
    app.run();
  }
  app.shutdown();

  return 0;
}
