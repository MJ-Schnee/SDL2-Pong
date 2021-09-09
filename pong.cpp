#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>

const int WINDOW_WIDTH = 904, WINDOW_HEIGHT = 800;
const int PADDLE_SPACING_FROM_EDGE = 45;
const int PADDLE_HEIGHT = WINDOW_HEIGHT * 0.05, PADDLE_WIDTH = WINDOW_WIDTH * 0.01;
const int BALL_RADIUS = WINDOW_HEIGHT * 0.02;
const char* SCORE_FONT_LOCATION = "./src/fonts/pong-score.ttf";

// SDL variables
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Event event;
TTF_Font *scoreFont;

struct Paddle {
  SDL_Rect rect {0, 0, PADDLE_WIDTH, PADDLE_HEIGHT};
  int score = 0;
} paddleLeft, paddleRight;

struct Ball {
  SDL_Rect rect {0, 0, BALL_RADIUS, BALL_RADIUS};
  float velX = 0, velY = 0;
} ball;

// Draws the black back screen and net
void drawBackground() {
  // Draw the black screen
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  // Draw the net
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  int netRectSpace = WINDOW_HEIGHT / 30; // There are 30 rectangles to represent the net
  SDL_Rect netRect {WINDOW_WIDTH / 2, 0, 3, 12};
  for (int y = 0; y < WINDOW_HEIGHT; y += netRectSpace) {
    netRect.y = y;
    SDL_RenderFillRect(renderer, &netRect);
  }
}

// Draws the paddles, ball, and scores
void drawGameObjects() {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  
  // Draw paddles and ball
  SDL_RenderFillRect(renderer, &paddleLeft.rect);
  SDL_RenderFillRect(renderer, &paddleRight.rect);
  SDL_RenderFillRect(renderer, &ball.rect);

  // Draw the scores
  const char* scoreTextLeft = std::to_string(paddleLeft.score).c_str();
  const char* scoreTextRight = std::to_string(paddleLeft.score).c_str();
  SDL_Surface* scoreSurfaceLeft = 
    TTF_RenderText_Solid(scoreFont, scoreTextLeft, {255, 255, 255, 255});
  SDL_Surface* scoreSurfaceRight = 
    TTF_RenderText_Solid(scoreFont, scoreTextRight, {255, 255, 255, 255});
  // It's easier to render the text as a texture rather than a surface
  SDL_Texture* scoreTextureLeft = SDL_CreateTextureFromSurface(renderer, scoreSurfaceLeft);
  SDL_Texture* scoreTextureRight = SDL_CreateTextureFromSurface(renderer, scoreSurfaceRight);
  int scoreDistFromTop = 32, scoreWidth = 73, scoreHeight = 100;
  SDL_Rect scoreRectLeft {217, scoreDistFromTop, scoreWidth, scoreHeight};
  SDL_Rect scoreRectRight {811, scoreDistFromTop, scoreWidth, scoreHeight};
  SDL_RenderCopy(renderer, scoreTextureLeft, NULL, &scoreRectLeft);
  SDL_RenderCopy(renderer, scoreTextureRight, NULL, &scoreRectRight);
  // Free the surface and destroy the texture since they aren't needed anymore
  SDL_FreeSurface(scoreSurfaceLeft);
  SDL_FreeSurface(scoreSurfaceRight);
  SDL_DestroyTexture(scoreTextureLeft);
  SDL_DestroyTexture(scoreTextureRight);
}

int main(int argc, char *argv[]) {
  // Initializations
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cout << "SDL Video Initialization Failed";
    return 1;
  }
  if (TTF_Init() != 0) {
    std::cout << "TTF Initialization Failed";
    return 1;
  }


  window = SDL_CreateWindow(
    "Pong",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    WINDOW_WIDTH, WINDOW_HEIGHT,
    0
  );
  if (!window) {
    std::cout << "SDL Window Creation Failed";
    return 1;
  }

  renderer = SDL_CreateRenderer(window, -1, 0);
  if (!renderer) {
    std::cout << "SDL Renderer Creation Failed";
    return 1;
  }

  scoreFont = TTF_OpenFont(SCORE_FONT_LOCATION, 24);
  if (!scoreFont) {
    std::cout << "Opening Font File " << SCORE_FONT_LOCATION << " Failed";
    return 1;
  }


  paddleLeft.rect.x = PADDLE_SPACING_FROM_EDGE;
  paddleLeft.rect.y = WINDOW_HEIGHT / 2 - PADDLE_HEIGHT / 2;
  
  paddleRight.rect.x = WINDOW_WIDTH - PADDLE_SPACING_FROM_EDGE - PADDLE_WIDTH;
  paddleRight.rect.y = paddleLeft.rect.y;
  
  ball.rect.x = WINDOW_WIDTH / 2 - BALL_RADIUS / 2;
  ball.rect.y = WINDOW_HEIGHT / 2 - BALL_RADIUS / 2;
  

  // Main game loop
  bool isPlaying = true;
  while (isPlaying) {
    while (SDL_PollEvent(&event) != 0) {
      if (event.type == SDL_QUIT) {
        isPlaying = false;
        // Clean up
        TTF_CloseFont(scoreFont);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
      }
    }
    
    drawBackground();
    drawGameObjects();
    SDL_RenderPresent(renderer);
  }
 
  return 0;
}