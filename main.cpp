#include "application.h"
#include <SDL.h>
#include <iostream>
#include <memory>

using namespace huigame;
using namespace std;

int main() {
  shared_ptr<VulkanApplication> app = make_shared<SDLApplication>();
  auto win = app->createVulkanWindow("title");
  auto extensions = win->getExtensionNames();
  for (auto e : *extensions) {
    cout << e << endl;
  }
  while (app->loop([](void) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        return false;
      }
    }
    return true;
  })) {
  }
  return 0;
}