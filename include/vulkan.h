#pragma once

#include <SDL_vulkan.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace huigame {
class Vulkan {
private:
  VkInstance instance;

public:
  Vulkan(const std::vector<char *> extensions) {
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    createInfo.enabledExtensionCount = (uint32_t)extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
    auto err = vkCreateInstance(&createInfo, nullptr, &this->instance);
    if (err < 0) {
      throw std::runtime_error("failed to create instance");
    }
  }
};
} // namespace huigame