# Pong
## Screenshots
Just a placeholder for now, but here is a video of the original I am trying to clone
[![IMAGE ALT TEXT](http://img.youtube.com/vi/fhd7FfGCdCo/0.jpg)](http://www.youtube.com/watch?v=fhd7FfGCdCo "Pong 1972 by Atari")

## Description
A clone of the 1972 game for the Atari 2600: Pong. This was made in C++ using the SDL2 library.
The main purpose of this project is to learn how to use SDL2 for simple 2D game development.
The game will be one player against an AI or against another player locally.

## Gameplay
Both players are bound by the screen and can only move vertically.
The ball will bounce off the top and bottom edges of the screen and the players' paddles.
A point is scored anytime a ball touches the left and right edges of the screen for the opposite side it hit.
Player 1 is the left paddle and player 2 or the AI is the right paddle.

**Player 1 Controls:**
| Button | Action |
| ------ | ----- |
| W | Move up |
| D | Move down |

**Player 2 Controls:**
| Button | Action |
| ------ | ----- |
| ↑ | Move up |
| ↓ | Move down |

## Usage
If you want to download and play on your own, there is currently only support for windows.
Ensure that MinGW is installed with C++ compilation then run `pong.bat` and launch `pong.exe`.