#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>
#include <time.h>
#include <chrono>
#include <algorithm>

const int WINDOW_WIDTH = 904, WINDOW_HEIGHT = 800;
const int PADDLE_SPACING_FROM_EDGE = 45;
const float PADDLE_HEIGHT = WINDOW_HEIGHT * 0.05f, PADDLE_WIDTH = WINDOW_WIDTH * 0.01f;
const float BALL_RADIUS = WINDOW_HEIGHT * 0.02f;
const char* SCORE_FONT_LOCATION = "./src/fonts/pong-score.ttf";
const float PADDLE_SPEED = 1.15f;

// SDL variables
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Event event;
TTF_Font *scoreFont;

struct Paddle {
  SDL_FRect rect {0.0f, WINDOW_HEIGHT / 2.0f - PADDLE_HEIGHT / 2.0f, PADDLE_WIDTH, PADDLE_HEIGHT};
  float velocity = 0.0f;
  int score = 0;
} paddleLeft, paddleRight;

struct Ball {
  SDL_FRect rect {
    WINDOW_WIDTH / 2.0f - BALL_RADIUS / 2.0f,
    WINDOW_HEIGHT / 2.0f - BALL_RADIUS / 2.0f,
    BALL_RADIUS, BALL_RADIUS
  };
  float velX = 0.0f, velY = 0.0f;
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
  SDL_RenderFillRectF(renderer, &paddleLeft.rect);
  SDL_RenderFillRectF(renderer, &paddleRight.rect);
  SDL_RenderFillRectF(renderer, &ball.rect);

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

void updatePaddlePosition(Paddle* paddle, float delta_time) {
    paddle->rect.y -= 
      paddle->velocity * delta_time + paddle->velocity * delta_time * delta_time * 0.5;
    if (paddle->rect.y > WINDOW_HEIGHT - PADDLE_HEIGHT) {
      paddle->rect.y = WINDOW_HEIGHT - PADDLE_HEIGHT;
    } else if (paddle->rect.y < 0) {
      paddle->rect.y = 0;
    }
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
  paddleRight.rect.x = WINDOW_WIDTH - PADDLE_SPACING_FROM_EDGE - PADDLE_WIDTH;


  // Main game loop
  bool isPlaying = true;
  float delta_time = 0.0f;
  while (isPlaying) {
    auto startTime = std::chrono::high_resolution_clock::now();

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        isPlaying = false;
        // Clean up
        TTF_CloseFont(scoreFont);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
      } else if (event.type == SDL_KEYDOWN) { // Change direction of paddle
        switch (event.key.keysym.sym) {
          case SDLK_w:
            paddleLeft.velocity = PADDLE_SPEED;
            break;
          case SDLK_s:
            paddleLeft.velocity = -PADDLE_SPEED;
            break;
          case SDLK_UP:
            paddleRight.velocity = PADDLE_SPEED;
            break;
          case SDLK_DOWN:
            paddleRight.velocity = -PADDLE_SPEED;
            break;
        }
      } else if (event.type == SDL_KEYUP) { // Halt movement of paddle
        switch (event.key.keysym.sym) {
          case SDLK_w:
          case SDLK_s:
            paddleLeft.velocity = 0.0f;
            break;
          case SDLK_UP:
          case SDLK_DOWN:
            paddleRight.velocity = 0.0f;
            break;
        }
      }
    }

    // Update game object positions
    updatePaddlePosition(&paddleLeft, delta_time);
    updatePaddlePosition(&paddleRight, delta_time);

    drawBackground();
    drawGameObjects();
    SDL_RenderPresent(renderer);

    auto stopTime = std::chrono::high_resolution_clock::now();
	  delta_time = 
      std::chrono::duration<float, std::chrono::milliseconds::period>(stopTime - startTime).count();
  }

  return 0;
}