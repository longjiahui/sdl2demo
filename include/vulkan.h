#pragma once

#include <SDL_vulkan.h>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
              uint64_t obj, size_t location, int32_t code,
              const char *layerPrefix, const char *msg, void *userData) {
  std::cerr << "validation layer: " << msg << std::endl;
  return VK_FALSE;
}

namespace huigame {
class Vulkan {
private:
  using Device = struct {
    VkDevice device;
    VkQueue queue;
  };

  std::shared_ptr<VkInstance> instance;
  std::shared_ptr<VkDebugReportCallbackEXT> debugReportCallbackInstance;
  // no need to cleanup for VkPhysicalDevice
  VkPhysicalDevice physicalDevice;
  // once pick physicalDevice, calculate the index
  std::optional<unsigned short> _graphicsQueueFamilyIndex;

  std::shared_ptr<std::vector<VkPhysicalDevice>> enumeratePhysicalDevices() {
    auto physicalDevices = std::make_shared<std::vector<VkPhysicalDevice>>();
    uint32_t gpuAmount = 0;
    auto err = vkEnumeratePhysicalDevices(*this->instance, &gpuAmount, nullptr);
    if (err < 0) {
      throw std::runtime_error("failed to enumerate physical devices");
    }
    physicalDevices->resize(gpuAmount);
    err = vkEnumeratePhysicalDevices(*this->instance, &gpuAmount,
                                     physicalDevices->data());
    if (err < 0) {
      throw std::runtime_error("failed to enumerate physical devices");
    }
    return physicalDevices;
  }

  using Extensions = std::vector<char *>;
  using Layers = std::vector<char *>;

  std::shared_ptr<VkInstance> createInstance(Extensions extensions,
                                             Layers layers) {
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    createInfo.enabledExtensionCount = (uint32_t)extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = (uint32_t)layers.size();
    createInfo.ppEnabledLayerNames = layers.data();
    auto debugReportCallbackCreateInfo =
        this->createDebugReportCallbackCreateInfo();
    createInfo.pNext = &debugReportCallbackCreateInfo;
    VkInstance instance;
    auto err = vkCreateInstance(&createInfo, nullptr, &instance);
    if (err < 0) {
      throw std::runtime_error("failed to create instance");
    }
    auto ret =
        std::shared_ptr<VkInstance>(new VkInstance(instance), [](auto p) {
          vkDestroyInstance(*p, nullptr);
        });
    return ret;
  }

  VkPhysicalDevice getPhysicalDevice() {
    auto physicalDevices = this->enumeratePhysicalDevices();
    // If a number >1 of GPUs got reported, find discrete GPU if present, or use
    // first one available. This covers most common cases
    // (multi-gpu/integrated+dedicated graphics). Handling more complicated
    // setups (multiple dedicated GPUs) is out of scope of this sample.
    VkPhysicalDevice finalD;
    for (auto &d : *physicalDevices) {
      VkPhysicalDeviceProperties p;
      vkGetPhysicalDeviceProperties(d, &p);
      if (p.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        finalD = d;
      }
    }
    if (finalD == nullptr) {
      // or just return the first one
      if (physicalDevices->size() > 0) {
        finalD = physicalDevices->at(0);
      } else {
        throw std::runtime_error("no physical devices found");
      }
    }
    return finalD;
  }

  void setPhysicalDevice(VkPhysicalDevice physicalDevice) {
    this->physicalDevice = physicalDevice;
    uint32_t amount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &amount, nullptr);
    std::vector<VkQueueFamilyProperties> properties(amount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &amount,
                                             properties.data());
    for (unsigned int i = 0; i < properties.size(); ++i) {
      auto p = properties.at(i);
      if (p.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        this->_graphicsQueueFamilyIndex = i;
      }
    }
  }

  // 获取vkCreateDebugReportCallbackEXT函数
  PFN_vkCreateDebugReportCallbackEXT
  getVkCreateDebugReportCallbackEXTFunction() {
    auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
        *this->instance, "vkCreateDebugReportCallbackEXT");
    if (func != nullptr) {
      return func;
    } else {
      throw std::runtime_error(
          "failed to get vkCreateDebugReportCallbackEXT function");
    }
  }
  PFN_vkDestroyDebugReportCallbackEXT
  getVkDestroyDebugReportCallbackEXTFunction() {
    auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
        *this->instance, "vkDestroyDebugReportCallbackEXT");
    if (func != nullptr) {
      return func;
    } else {
      throw std::runtime_error(
          "failed to get vkDestroyDebugReportCallbackEXT function");
    }
  }

  VkDebugReportCallbackCreateInfoEXT createDebugReportCallbackCreateInfo() {
    VkDebugReportCallbackCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags =
        -VK_DEBUG_REPORT_ERROR_BIT_EXT | -VK_DEBUG_REPORT_WARNING_BIT_EXT;
    createInfo.pfnCallback = debugCallback;
    return createInfo;
  }

  std::shared_ptr<VkDebugReportCallbackEXT> getDebugReportCallbackInstance() {
    if (!this->instance) {
      throw std::runtime_error("instance is not initialized");
    }
    VkDebugReportCallbackEXT debugReportCallbackInstance;
    auto createReportCallbackEXT =
        this->getVkCreateDebugReportCallbackEXTFunction();
    auto createInfo = this->createDebugReportCallbackCreateInfo();
    createReportCallbackEXT(*this->instance, &createInfo, nullptr,
                            &debugReportCallbackInstance);
    return std::shared_ptr<VkDebugReportCallbackEXT>(
        new VkDebugReportCallbackEXT(debugReportCallbackInstance),
        [this](auto p) {
          auto destroy = this->getVkDestroyDebugReportCallbackEXTFunction();
          destroy(*this->instance, *p, nullptr);
        });
  }

  std::shared_ptr<Device> createDevice() {
    auto d = std::make_shared<Device>();
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    if (!this->_graphicsQueueFamilyIndex.has_value()) {
      throw std::runtime_error("graphics queue family index is not set");
    }
    queueCreateInfo.queueFamilyIndex = this->_graphicsQueueFamilyIndex.value();
    queueCreateInfo.queueCount = 1;
    float queuePriorities[1] = {0.0};
    queueCreateInfo.pQueuePriorities = queuePriorities;
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    vkCreateDevice(this->physicalDevice, &createInfo, nullptr, &d->device);
    vkGetDeviceQueue(d->device, this->_graphicsQueueFamilyIndex.value(), 0,
                     &d->queue);
    return d;
  }

public:
  Vulkan(Extensions extensions) {

    // 开启校验层
    Layers layers = {"VK_LAYER_KHRONOS_validation"};
    extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    this->instance = this->createInstance(extensions, layers);

    // 设置校验回调函数
    this->debugReportCallbackInstance = this->getDebugReportCallbackInstance();

    this->setPhysicalDevice(this->getPhysicalDevice());
  }
};
} // namespace huigame