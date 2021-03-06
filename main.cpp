#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <string>
#include <time.h>
#include <chrono>
#include <algorithm>
#include <tgmath.h>

#define PI 3.14159265

const int WINDOW_WIDTH = 904, WINDOW_HEIGHT = 800;
const char* SCORE_FONT_LOCATION = "./src/fonts/pong-score.ttf";
const char* SFX_PADDLE_LOCATION = "./src/sfx/pong-paddle.wav";
const char* SFX_SCORE_LOCATION = "./src/sfx/pong-score.wav";
const char* SFX_WALL_LOCATION = "./src/sfx/pong-wall.wav";
const float PADDLE_SPACING_FROM_EDGE = 45.0f;
const float PADDLE_HEIGHT = WINDOW_HEIGHT * 0.07f, PADDLE_WIDTH = WINDOW_WIDTH * 0.01f;
const float PADDLE_SPEED = 0.7f;
const float BALL_RADIUS = WINDOW_HEIGHT * 0.01f;
const float BALL_SPEED = 0.5f;
const float PADDLE_SPAWN_Y = WINDOW_HEIGHT / 2.0f - PADDLE_HEIGHT / 2.0f;

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Event event;
TTF_Font* scoreFont;
Mix_Chunk* soundHitPaddle;
Mix_Chunk* soundScore;
Mix_Chunk* soundHitWall;
bool leftSideServing;
bool ballRespawning = false;
float ballRespawnTime = 0.0f; // This keeps track of the time when the ball will respawn
bool player2Ai = true;

struct Paddle {
  SDL_FRect rect {0.0f, PADDLE_SPAWN_Y, PADDLE_WIDTH, PADDLE_HEIGHT};
  float velocity = 0.0f;
  int score = 0;
} paddleLeft, paddleRight;

struct Ball {
  SDL_FRect rect {
    WINDOW_WIDTH / 2.0f - BALL_RADIUS,
    WINDOW_HEIGHT / 2.0f - BALL_RADIUS,
    BALL_RADIUS * 2.0f, BALL_RADIUS * 2.0f
  };
  float velX = 0.0f, velY = 0.0f;
} ball;

// Draws the background, net paddles, ball, and scores
void drawGame(bool renderPaddles) {
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

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  
  // Draw paddles and ball
  if (renderPaddles) {
    SDL_RenderFillRectF(renderer, &paddleLeft.rect);
    SDL_RenderFillRectF(renderer, &paddleRight.rect);
  }
  SDL_RenderFillRectF(renderer, &ball.rect);

  // Draw the scores
  const char* scoreTextLeft = std::to_string(paddleLeft.score).c_str();
  const char* scoreTextRight = std::to_string(paddleRight.score).c_str();
  SDL_Surface* scoreSurfaceLeft = 
    TTF_RenderText_Solid(scoreFont, scoreTextLeft, {255, 255, 255, 255});
  SDL_Surface* scoreSurfaceRight = 
    TTF_RenderText_Solid(scoreFont, scoreTextRight, {255, 255, 255, 255});
  // It's easier to render the text as a texture rather than a surface
  SDL_Texture* scoreTextureLeft = SDL_CreateTextureFromSurface(renderer, scoreSurfaceLeft);
  SDL_Texture* scoreTextureRight = SDL_CreateTextureFromSurface(renderer, scoreSurfaceRight);
  int scoreDistFromTop = 32, scoreWidth = 73, scoreHeight = 100;
  SDL_Rect scoreRectLeft {
    paddleLeft.score < 10 ? 273 : 273 - scoreWidth, scoreDistFromTop,
    paddleLeft.score < 10 ? scoreWidth : 2 * scoreWidth, scoreHeight
  };
  SDL_Rect scoreRectRight {
    paddleRight.score < 10 ? 811 : 811 - scoreWidth, scoreDistFromTop,
    paddleRight.score < 10 ? scoreWidth : 2 * scoreWidth, scoreHeight
  };
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
      paddle->velocity * delta_time;
    if (paddle->rect.y > WINDOW_HEIGHT - PADDLE_HEIGHT) {
      paddle->rect.y = WINDOW_HEIGHT - PADDLE_HEIGHT;
    } else if (paddle->rect.y < 0) {
      paddle->rect.y = 0;
    }
}

void updateBallPosition(float delta_time) {
  ball.rect.x += ball.velX * delta_time;
  ball.rect.y -= ball.velY * delta_time;
}

// Returns if 2 floating point Rects are colliding
// Needed because SDL_HasIntersection only works with integer Rects
bool areColliding(SDL_FRect r1, SDL_FRect r2) {
  return
    r1.x < r2.x + r2.w &&
    r1.x + r1.w > r2.x &&
    r1.y < r2.y + r2.h &&
    r1.y + r1.h > r2.y;
}

// Respawns the ball at a random point with random velocity on the net after 3 seconds
void respawnBall() {
  // Set the ball off screen and set the ball respawn timer
  if (!ballRespawning) {
    ball.velX = 0;
    ball.velY = 0;
    ball.rect.x = -50;
    ball.rect.y = -50;
    ballRespawnTime = 3000;
    ballRespawning = true;
  } else { // Spawn in the ball
    ballRespawning = false;
    int randomNumber = rand();
    float angle = (randomNumber % 90 - 45) * PI / 180.0f; // -45 deg. to 45 deg. (prevents vertical start)
    ball.velX = cos(angle) * BALL_SPEED * (leftSideServing ? 1 : -1);
    ball.velY = sin(angle) * BALL_SPEED;
    ball.rect.x = WINDOW_WIDTH / 2.0f - BALL_RADIUS;
    ball.rect.y = randomNumber % int(WINDOW_HEIGHT - BALL_RADIUS * 2) + BALL_RADIUS * 2.0f;
  }
}

// Adjust ball velocity if it hits a paddle
void paddleHitBall(bool leftPaddle) {
  // How far from center of the paddle is the middle of the ball
  float relativeBallPaddle = 
    ((leftPaddle ? paddleLeft.rect.y : paddleRight.rect.y) + PADDLE_HEIGHT / 2)
    - (ball.rect.y + BALL_RADIUS);
  // Either edge of paddle is 1, middle of paddle is 0
  float normalizedBallPaddle = relativeBallPaddle / (PADDLE_HEIGHT / 2);
  float angle = normalizedBallPaddle * 45 * PI / 180.0f; // 75 deg is the max angle we want
  // Ball goes faster if hit on edge, slower if in center
  float speedMultiplier = 0.5 * sin(3 * normalizedBallPaddle - PI / 2) + 1.2;
  ball.velX = speedMultiplier * BALL_SPEED * cos(angle) * (leftPaddle ? 1 : -1);
  ball.velY = speedMultiplier * BALL_SPEED * sin(angle);
}

// Act based on if the ball collided with something
void ballCollision(bool playing) {
  if (playing && areColliding(ball.rect, paddleLeft.rect)) {
    Mix_PlayChannel(-1, soundHitPaddle, 0);
    paddleHitBall(true);
  }
  else if (playing && areColliding(ball.rect, paddleRight.rect)) {
    Mix_PlayChannel(-1, soundHitPaddle, 0);
    paddleHitBall(false);
  }
  else if (ball.rect.y + BALL_RADIUS * 2 > WINDOW_HEIGHT || ball.rect.y < 0) { // Top or bottom of screen
    if (playing) Mix_PlayChannel(-1, soundHitWall, 0);
    ball.velY *= -1;
    ball.rect.y += ball.velY > 0 ? -1 : 1;
  }
  else if (ball.rect.x < 0) { // Left side of screen
    if (playing) {
      ++paddleRight.score;
      Mix_PlayChannel(-1, soundScore, 0);
      leftSideServing = false;
      respawnBall();
    } else {
      ball.velX *= -1;
      ball.rect.x -= ball.velX > 0 ? -1 : 1;
    }
  }
  else if (ball.rect.x + BALL_RADIUS * 2 > WINDOW_WIDTH) { // Right side of screen
    if (playing) {
      ++paddleLeft.score;
      Mix_PlayChannel(-1, soundScore, 0);
      leftSideServing = true;
      respawnBall();
    } else {
      ball.velX *= -1;
      ball.rect.x -= ball.velX > 0 ? -1 : 1;
    }
  }
}

// Resets scores, serve, ball, and AI
void restartGame() {
  paddleLeft.score = 0;
  paddleLeft.rect.y = PADDLE_SPAWN_Y;
  paddleRight.score = 0;
  paddleRight.rect.y = PADDLE_SPAWN_Y;
  leftSideServing = rand() % 2; // Random initial serve
  ballRespawning = false;
  player2Ai = true;
  respawnBall();
}

int main(int argc, char *argv[]) {
  // Initializations
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    std::cout << "SDL Video or Audio Initialization Failed\n" << SDL_GetError();
    return 1;
  }
  if (TTF_Init() != 0) {
    std::cout << "TTF Initialization Failed\n" << TTF_GetError();
    return 1;
  }
  if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
    std::cout << "SDL Mixer Audio Initialization Failed\n" << Mix_GetError();
    return 1;
  }
  srand(time(0));

  // SDL variable assignments
  window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  if (!window) {
    std::cout << "SDL Window Creation Failed\n" << SDL_GetError();
    return 1;
  }
  renderer = SDL_CreateRenderer(window, -1, 0);
  if (!renderer) {
    std::cout << "SDL Renderer Creation Failed\n" << SDL_GetError();
    return 1;
  }
  scoreFont = TTF_OpenFont(SCORE_FONT_LOCATION, 24);
  if (!scoreFont) {
    std::cout << "Opening Font File " << SCORE_FONT_LOCATION << " Failed\n" << TTF_GetError();
    return 1;
  }
  soundHitPaddle = Mix_LoadWAV(SFX_PADDLE_LOCATION);
  soundHitWall = Mix_LoadWAV(SFX_WALL_LOCATION);
  soundScore = Mix_LoadWAV(SFX_SCORE_LOCATION);
  if (soundHitPaddle == NULL || soundHitWall == NULL || soundScore == NULL) {
    std::cout << "Loading WAV sound files Failed\n" << Mix_GetError();
    return 1;
  }

  // Initialize the game objects
  paddleLeft.rect.x = PADDLE_SPACING_FROM_EDGE;
  paddleRight.rect.x = WINDOW_WIDTH - PADDLE_SPACING_FROM_EDGE - PADDLE_WIDTH;
  leftSideServing = rand() % 2; // Random initial serve
  respawnBall();

  bool gameRunning = true;
  bool gameOver = false;
  float delta_time = 0.0f;
  while (gameRunning) {
    auto startTime = std::chrono::high_resolution_clock::now();

    if (!gameOver) { // Gameplay
      // Handle Input
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
          gameRunning = false;
        } else if (event.type == SDL_KEYDOWN) {
          switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
              gameRunning = false;
              break;
            case SDLK_r:
              restartGame();
              break;
            case SDLK_w:
              paddleLeft.velocity = PADDLE_SPEED;
              break;
            case SDLK_s:
              paddleLeft.velocity = -PADDLE_SPEED;
              break;
            case SDLK_UP:
              if (!player2Ai) paddleRight.velocity = PADDLE_SPEED;
              break;
            case SDLK_DOWN:
              if (!player2Ai) paddleRight.velocity = -PADDLE_SPEED;
              break;
          }
        } else if (event.type == SDL_KEYUP) {
          switch (event.key.keysym.sym) {
            case SDLK_w:
            case SDLK_s:
              paddleLeft.velocity = 0.0f;
              break;
            case SDLK_UP:
            case SDLK_DOWN:
              if (!player2Ai) paddleRight.velocity = 0.0f;
              break;
            case SDLK_a:
              player2Ai = !player2Ai;
              paddleRight.velocity = 0.0f;
              break;
          }
        }
      }

      if (ballRespawning) {
        ballRespawnTime -= delta_time;
        if (ballRespawnTime < 0) {
          respawnBall();
        }
      } else {
        ballCollision(true);
      }

      updatePaddlePosition(&paddleLeft, delta_time);
      if (player2Ai) {
        float rightPaddleBallVertDist = (paddleRight.rect.y + PADDLE_HEIGHT / 2) - (ball.rect.y + BALL_RADIUS);
        if (ball.rect.y < 0) rightPaddleBallVertDist = 0.0f;
        float speedMultAi = 0.7f;
        if (abs(rightPaddleBallVertDist) > 3) {
          paddleRight.velocity = speedMultAi * (rightPaddleBallVertDist > 1 ? PADDLE_SPEED : -PADDLE_SPEED);
        } else {
          paddleRight.velocity = 0.0f;
        }
      }
      updatePaddlePosition(&paddleRight, delta_time);
      updateBallPosition(delta_time);

      drawGame(true);
      SDL_RenderPresent(renderer);
      
      if (paddleLeft.score >= 11 || paddleRight.score >= 11) {
        ballRespawning = true;
        respawnBall();
        gameOver = true;
      }
    } else { // Game over screen
      // Handle Input
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
          gameRunning = false;
        } else if (event.type == SDL_KEYDOWN) {
          switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
              gameRunning = false;
              break;
            case SDLK_r:
              restartGame();
              gameOver = false;
              break;
          }
        }
      }
      
      ballCollision(false);

      updateBallPosition(delta_time);

      drawGame(false);
      SDL_RenderPresent(renderer);
    }
    
    auto stopTime = std::chrono::high_resolution_clock::now();
    delta_time = 
      std::chrono::duration<float, std::chrono::milliseconds::period>(stopTime - startTime).count();
  }

  TTF_CloseFont(scoreFont);
  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();

  return 0;
}