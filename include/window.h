#pragma once

#include <SDL.h>
#include <SDL_vulkan.h>
#include <functional>
#include <iostream>
#include <memory>
#include <string.h>
#include <vulkan/vulkan.h>

namespace huigame {

class WindowHandler {
public:
  virtual void *createWindow(const char *) = 0;
  virtual void destroyWindow(void *) = 0;
  WindowHandler() {};
  virtual ~WindowHandler() {};
  // virtual void loop(std::function<void(void *)>) = 0;
};

class Window {
protected:
  // 隐藏底层窗口库细节
  void *instance;
  std::shared_ptr<WindowHandler> handler;

public:
  Window(std::shared_ptr<WindowHandler> handler, const char *title) {
    this->handler = handler;
    this->handler->createWindow(title);
  }
  ~Window() { this->handler->destroyWindow(this->instance); }
  Window(const Window &obj) = delete;  // 拷贝构造函数
  Window(const Window &&obj) = delete; // 拷贝构造函数
};

class VulkanWindow : public Window {
protected:
public:
  VulkanWindow(std::shared_ptr<WindowHandler> handler, const char *title)
      : Window(handler, title) {}
  virtual std::shared_ptr<std::vector<char *>> getExtensionNames() = 0;
};

class SDLWindowHandler : WindowHandler {
public:
  SDLWindowHandler() {}
  ~SDLWindowHandler() {}
  void *createWindow(const char *title) {
    return SDL_CreateWindow(
        title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480,
        SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE |
            SDL_WINDOW_ALLOW_HIGHDPI);
  }
  void destroyWindow(void *instance) {
    SDL_DestroyWindow((SDL_Window *)instance);
  }
};

static std::shared_ptr<std::vector<VkExtensionProperties>>
enumerateExtensionProperties() {
  // Enumerate available extensions
  uint32_t count;
  auto properties = std::make_shared<std::vector<VkExtensionProperties>>();
  vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
  properties->resize(count);
  auto err = vkEnumerateInstanceExtensionProperties(nullptr, &count,
                                                    properties->data());
  if (err < 0) {
    throw std::runtime_error(
        "failed to enumerate instance extension properties");
  }
  return properties;
}

class SDLWindow : public VulkanWindow {
public:
  SDLWindow(const char *title)
      : VulkanWindow(std::shared_ptr<WindowHandler>(
                         (WindowHandler *)new SDLWindowHandler),
                     title) {}

  virtual std::shared_ptr<std::vector<char *>> getExtensionNames() {
    auto window = (SDL_Window *)this->instance;
    auto extensions = std::make_shared<std::vector<char *>>();
    uint32_t count = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
    extensions->resize(count);
    std::cout << count << std::endl;
    SDL_Vulkan_GetInstanceExtensions(window, &count,
                                     (const char **)extensions->data());

    auto properties = enumerateExtensionProperties();
    for (const auto p : *properties) {
      if (strcmp(p.extensionName,
                 VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0) {
        extensions->push_back(
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
      }
      if (strcmp(p.extensionName,
                 VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) == 0) {
        extensions->push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
      }
    }
    return extensions;
  }
};
} // namespace huigame