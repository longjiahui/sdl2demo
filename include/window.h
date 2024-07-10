#pragma once

#include <SDL.h>
#include <functional>
#include <memory>

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

class SDLWindowHandler : WindowHandler {
public:
  SDLWindowHandler() {}
  ~SDLWindowHandler() {}
  void *createWindow(const char *title) {
    return SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
  }
  void destroyWindow(void *instance) {
    SDL_DestroyWindow((SDL_Window *)instance);
  }
};

class SDLWindow : public Window {
public:
  SDLWindow(const char *title)
      : Window(std::shared_ptr<WindowHandler>(
                   (WindowHandler *)new SDLWindowHandler),
               title) {}

protected:
  void *createWindow(const char *title) {
    return SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
  }
  void destroyWindow(void *instance) {
    SDL_DestroyWindow((SDL_Window *)instance);
  }
};
} // namespace huigame