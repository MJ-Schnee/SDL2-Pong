#include <SDL2/SDL.h>
#include <iostream>

// Close to 1972 Pong's screen ratio while being big enough for modern monitors
const int WINDOW_WIDTH = 904, WINDOW_HEIGHT = 800;

// Draws the black back screen and net
void drawBackground(SDL_Renderer* renderer) {
  // Draw the black screen
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  // Draw the net
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  int netRectSpace = WINDOW_HEIGHT / 30; // There are 30 rectangles to represent the net
  SDL_Rect netRect = {WINDOW_WIDTH / 2, 0, 3, 12}; // X, Y, Width, Height
  for (int y = 0; y < WINDOW_HEIGHT; y += netRectSpace) {
    netRect.y = y;
    SDL_RenderFillRect(renderer, &netRect);
  }

  SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[]) {
  // SDL variables
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Event event;

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cout << "SDL Video Initialization Failed";
    return 1;
  }

  // Create the window and renderer
  window = SDL_CreateWindow(
    "Pong",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    WINDOW_WIDTH, WINDOW_HEIGHT,
    0
  );
  renderer = SDL_CreateRenderer(window, -1, 0);
  if (!window || !renderer) {
    std::cout << !window ? "SDL Window Creation Failed" : "SDL Renderer Creation Failed";
    return 1;
  }
  
  drawBackground(renderer);
  
  // Main game loop
  bool isPlaying = true;
  while (isPlaying) {
    while (SDL_PollEvent(&event) != 0) {
      if (event.type == SDL_QUIT) {
        isPlaying = false;
      }
    }
    
    drawBackground(renderer);
  }

  // Clean up
  SDL_DestroyWindow(window);
  SDL_Quit();
 
  return 0;
}