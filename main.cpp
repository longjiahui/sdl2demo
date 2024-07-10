#include "application.h"
#include <SDL.h>
#include <memory>

using namespace huigame;
using namespace std;

int main() {
  shared_ptr<Application> app = make_shared<SDLApplication>();
  auto win = app->createWindow("title");
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