#include <SDL.h>
#include <iostream>
#include <string>

using namespace std;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main() {
  SDL_Window *window = nullptr;
  SDL_Surface *surface = nullptr;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    throw "SDL_Init failed: " + string(SDL_GetError());
  }

  window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                            SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (window == nullptr) {
    throw "SDL_CreateWindow failed: " + string(SDL_GetError());
  }

  surface = SDL_GetWindowSurface(window);
  SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 0xFF, 0xFF, 0xFF));
  SDL_UpdateWindowSurface(window);

  // Hack to get window to stay up
  SDL_Event e;
  bool quit = false;
  while (quit == false) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT)
        quit = true;
    }
  }

  // Destroy window
  SDL_DestroyWindow(window);

  // Quit SDL subsystems
  SDL_Quit();

  return 0;
}