#pragma once

#include "window.h"

#include <SDL.h>
#include <functional>
#include <memory>
#include <stdexcept>

namespace huigame {

class Application {
public:
  virtual std::shared_ptr<Window> createWindow(const char *title) = 0;
  virtual bool loop(std::function<bool(void)>, unsigned short = 16) = 0;
};

class SDLApplication : public Application {
public:
  SDLApplication() : Application() {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
      throw std::runtime_error("SDL_Init failed");
    }
  };

  std::shared_ptr<Window> createWindow(const char *title) {
    return std::make_shared<SDLWindow>(title);
  }

  bool loop(std::function<bool(void)> loopFunc, unsigned short delay) {
    SDL_Delay(delay);
    return loopFunc();
  }
};
} // namespace huigame