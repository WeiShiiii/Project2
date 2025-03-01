/**
* Author: Wei Shi
* Assignment: Pong Clone
* Date due: 2025-3-01, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include <SDL2/SDL.h>
#include <iostream>

// Window size
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// Paddle properties
const int PADDLE_WIDTH = 10;
const int PADDLE_HEIGHT = 100;
const int PADDLE_SPEED = 300.0f; // Pixels per second

// Ball properties
const int BALL_SIZE = 10;
float ballX = WINDOW_WIDTH / 2, ballY = WINDOW_HEIGHT / 2;
float ballSpeedX = 300.0f, ballSpeedY = 300.0f; // Pixels per second

// Paddle positions
float paddle1Y = (WINDOW_HEIGHT / 2) - (PADDLE_HEIGHT / 2);
float paddle2Y = (WINDOW_HEIGHT / 2) - (PADDLE_HEIGHT / 2);

bool running = true;
bool singlePlayerMode = false;

Uint32 lastFrameTime = 0;

void handleInput(bool &wPressed, bool &sPressed, bool &upPressed, bool &downPressed) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_w: wPressed = true; break;
                case SDLK_s: sPressed = true; break;
                case SDLK_UP: upPressed = true; break;
                case SDLK_DOWN: downPressed = true; break;
                case SDLK_t: singlePlayerMode = !singlePlayerMode; break;
            }
        } else if (event.type == SDL_KEYUP) {
            switch (event.key.keysym.sym) {
                case SDLK_w: wPressed = false; break;
                case SDLK_s: sPressed = false; break;
                case SDLK_UP: upPressed = false; break;
                case SDLK_DOWN: downPressed = false; break;
            }
        }
    }
}

void update(bool wPressed, bool sPressed, bool upPressed, bool downPressed) {
    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - lastFrameTime) / 2000.0f;
    lastFrameTime = currentTime;

    if (wPressed && paddle1Y > 0) paddle1Y -= PADDLE_SPEED * deltaTime;
    if (sPressed && paddle1Y < WINDOW_HEIGHT - PADDLE_HEIGHT) paddle1Y += PADDLE_SPEED * deltaTime;
    if (!singlePlayerMode) {
        if (upPressed && paddle2Y > 0) paddle2Y -= PADDLE_SPEED * deltaTime;
        if (downPressed && paddle2Y < WINDOW_HEIGHT - PADDLE_HEIGHT) paddle2Y += PADDLE_SPEED * deltaTime;
    } else {
        if (ballY < paddle2Y + PADDLE_HEIGHT / 2) paddle2Y -= PADDLE_SPEED * deltaTime;
        else if (ballY > paddle2Y + PADDLE_HEIGHT / 2) paddle2Y += PADDLE_SPEED * deltaTime;
    }

    ballX += ballSpeedX * deltaTime;
    ballY += ballSpeedY * deltaTime;

    if (ballY <= 0 || ballY + BALL_SIZE >= WINDOW_HEIGHT) ballSpeedY = -ballSpeedY;
    if (ballX <= PADDLE_WIDTH && ballY > paddle1Y && ballY < paddle1Y + PADDLE_HEIGHT) ballSpeedX = -ballSpeedX;
    if (ballX >= WINDOW_WIDTH - PADDLE_WIDTH - BALL_SIZE && ballY > paddle2Y && ballY < paddle2Y + PADDLE_HEIGHT) ballSpeedX = -ballSpeedX;

    if (ballX < 0) {
        std::cout << "Game Over! Player 2 Wins!" << std::endl;
        running = false;
    }
    if (ballX > WINDOW_WIDTH) {
        std::cout << "Game Over! Player 1 Wins!" << std::endl;
        running = false;
    }
}

void render(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect paddle1 = {0, (int)paddle1Y, PADDLE_WIDTH, PADDLE_HEIGHT};
    SDL_Rect paddle2 = {WINDOW_WIDTH - PADDLE_WIDTH, (int)paddle2Y, PADDLE_WIDTH, PADDLE_HEIGHT};
    SDL_Rect ball = {(int)ballX, (int)ballY, BALL_SIZE, BALL_SIZE};
    SDL_RenderFillRect(renderer, &paddle1);
    SDL_RenderFillRect(renderer, &paddle2);
    SDL_RenderFillRect(renderer, &ball);
    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Pong Clone", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    lastFrameTime = SDL_GetTicks();
    bool wPressed = false, sPressed = false, upPressed = false, downPressed = false;
    
    while (running) {
        handleInput(wPressed, sPressed, upPressed, downPressed);
        update(wPressed, sPressed, upPressed, downPressed);
        render(renderer);
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
