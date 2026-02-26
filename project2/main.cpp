/**
* Author: Matthew Jiang 
* Assignment: Pong Variations
* Date due: 02/28/2026 
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// Enums
enum AppStatus { TERMINATED, RUNNING };

// Global Constants
constexpr int SCREEN_WIDTH  = 1800;
constexpr int SCREEN_HEIGHT = 1100;
constexpr int FPS           = 120;

constexpr float PADDLE_SPEED = 800.0f;
constexpr float BALL_SPEED   = 1000.0f;

constexpr int MAX_BALLS  = 3;
constexpr int WIN_SCORE  = 5;
constexpr char BG_COLOUR[] = "#F8F1C8";

AppStatus gAppStatus = RUNNING;

// Textures 
Texture2D gPaddleTexture;
Texture2D gBallTexture;
Texture2D gBackgroundTexture;     
Texture2D gP1WinTexture;          
Texture2D gP2WinTexture;          

float gPreviousTicks = 0.0f;

// Game State
bool gSinglePlayer = false;   // press T to toggle
bool gGameOver     = false;

int gScore1 = 0;
int gScore2 = 0;

int gActiveBalls = 1;

// Paddle state 
Vector2 gPaddleSize  = { 30.0f, 180.0f };
Vector2 gPaddle1Pos  = { 80.0f,  SCREEN_HEIGHT / 2.0f };
Vector2 gPaddle2Pos  = { SCREEN_WIDTH - 80.0f, SCREEN_HEIGHT / 2.0f };


struct Ball {
    Vector2 pos;    
    Vector2 vel;
    Vector2 size;
    bool active;
};

Ball gBalls[MAX_BALLS];

void initialise();
void processInput();
void update();
void render();
void shutdown();

Color ColorFromHex(const char *hex)
{
    // Skip leading '#', if present
    if (hex[0] == '#') hex++;

    // Default alpha = 255 (opaque)
    unsigned int r = 0, 
                 g = 0, 
                 b = 0, 
                 a = 255;

    // 6‑digit form: RRGGBB
    if (sscanf(hex, "%02x%02x%02x", &r, &g, &b) == 3) {
        return (Color){ (unsigned char) r,
                        (unsigned char) g,
                        (unsigned char) b,
                        (unsigned char) a };
    }

    // 8‑digit form: RRGGBBAA
    if (sscanf(hex, "%02x%02x%02x%02x", &r, &g, &b, &a) == 4) {
        return (Color){ (unsigned char) r,
                        (unsigned char) g,
                        (unsigned char) b,
                        (unsigned char) a };
    }

    // Fallback – return white so you notice something went wrong
    return RAYWHITE;
}

float clampf(float v, float lo, float hi);
bool isColliding(const Vector2 *posA, const Vector2 *sizeA,
                 const Vector2 *posB, const Vector2 *sizeB);
void resetBall(Ball &b, int dirSign, int index);
void restartGame(); 

float clampf(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

bool isColliding(const Vector2 *posA, const Vector2 *sizeA,
                 const Vector2 *posB, const Vector2 *sizeB)
{
    float xDistance = fabs(posA->x - posB->x) - ((sizeA->x + sizeB->x) / 2.0f);
    float yDistance = fabs(posA->y - posB->y) - ((sizeA->y + sizeB->y) / 2.0f);

    return (xDistance < 0.0f && yDistance < 0.0f);
}

void resetBall(Ball &b, int dirSign, int index)
{
    b.pos = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };

    float speed = BALL_SPEED + 80.0f * index;

    b.vel.x = speed * (float)dirSign;
    b.vel.y = (index % 2 == 0 ? 1.0f : -1.0f) * (260.0f + 40.0f * index);
}

void restartGame()
{
    gGameOver = false;
    gScore1 = 0;
    gScore2 = 0;

    gPaddle1Pos = { 80.0f,  SCREEN_HEIGHT / 2.0f };
    gPaddle2Pos = { SCREEN_WIDTH - 80.0f, SCREEN_HEIGHT / 2.0f };

    
    gActiveBalls = (int)clampf((float)gActiveBalls, 1.0f, (float)MAX_BALLS);

    for (int i = 0; i < MAX_BALLS; i++) {
        gBalls[i].size = { 64.0f, 64.0f };
        gBalls[i].active = (i < gActiveBalls);
        resetBall(gBalls[i], (i % 2 == 0 ? 1 : -1), i);
    }

    // Reset timing so the first frame after restart isn't huge
    gPreviousTicks = (float)GetTime();
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong Variations");

    
    gPaddleTexture     = LoadTexture("assets/patrick.jpg");
    gBallTexture       = LoadTexture("assets/eric.jpg");

    gBackgroundTexture = LoadTexture("assets/krustykrab.jpg");
    gP1WinTexture      = LoadTexture("assets/sponge.jpeg");
    gP2WinTexture      = LoadTexture("assets/fish.jpeg");

    // balls
    for (int i = 0; i < MAX_BALLS; i++)
    {
        gBalls[i].size   = { 64.0f, 64.0f };
        gBalls[i].active = (i == 0);
        resetBall(gBalls[i], (i % 2 == 0 ? 1 : -1), i);
    }
    gPreviousTicks = (float)GetTime();
    SetTargetFPS(FPS);
}

void processInput() {
    if (WindowShouldClose()) gAppStatus = TERMINATED;

    if (gGameOver && IsKeyPressed(KEY_R)) {
        restartGame();
        return;
    }

    // Toggle 1P/2P 
    if (!gGameOver && IsKeyPressed(KEY_T)) {
        gSinglePlayer = !gSinglePlayer;
    }

    // Choose number of balls 
    if (!gGameOver) {
        if (IsKeyPressed(KEY_ONE))   gActiveBalls = 1;
        if (IsKeyPressed(KEY_TWO))   gActiveBalls = 2;
        if (IsKeyPressed(KEY_THREE)) gActiveBalls = 3;

        gActiveBalls = (int)clampf((float)gActiveBalls, 1.0f, (float)MAX_BALLS);

        for (int i = 0; i < MAX_BALLS; i++) {
            gBalls[i].active = (i < gActiveBalls);
        }
    }
}

void update() 
{
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    if (gGameOver) return;

    // Paddle movement 
    if (IsKeyDown(KEY_W)) gPaddle1Pos.y -= PADDLE_SPEED * deltaTime;
    if (IsKeyDown(KEY_S)) gPaddle1Pos.y += PADDLE_SPEED * deltaTime;

    // Player 2 or AI 
    if (!gSinglePlayer) {
        if (IsKeyDown(KEY_UP))   gPaddle2Pos.y -= PADDLE_SPEED * deltaTime;
        if (IsKeyDown(KEY_DOWN)) gPaddle2Pos.y += PADDLE_SPEED * deltaTime;
    } else {
        // AI
        Ball *target = nullptr;
        for (int i = 0; i < MAX_BALLS; i++) {
            if (gBalls[i].active) { target = &gBalls[i]; break; }
        }

        if (target) {
            float deadzone = 12.0f;
            float dir = 0.0f;
            if (target->pos.y > gPaddle2Pos.y + deadzone) dir =  1.0f;
            if (target->pos.y < gPaddle2Pos.y - deadzone) dir = -1.0f;

            gPaddle2Pos.y += (PADDLE_SPEED * 0.85f) * dir * deltaTime;
        }
    }

    // Clamp paddles
    float halfPaddleH = gPaddleSize.y / 2.0f;
    gPaddle1Pos.y = clampf(gPaddle1Pos.y, halfPaddleH, SCREEN_HEIGHT - halfPaddleH);
    gPaddle2Pos.y = clampf(gPaddle2Pos.y, halfPaddleH, SCREEN_HEIGHT - halfPaddleH);

    // Balls update
    for (int i = 0; i < MAX_BALLS; i++)
    {
        if (!gBalls[i].active) continue;

        Ball &ball = gBalls[i];

        ball.pos.x += ball.vel.x * deltaTime;
        ball.pos.y += ball.vel.y * deltaTime;

        float halfBall = ball.size.y / 2.0f;

        // Bounce top/bottom (Requirement 1)
        if (ball.pos.y - halfBall <= 0.0f) {
            ball.pos.y = halfBall;
            ball.vel.y *= -1.0f;
        }
        if (ball.pos.y + halfBall >= SCREEN_HEIGHT) {
            ball.pos.y = SCREEN_HEIGHT - halfBall;
            ball.vel.y *= -1.0f;
        }

        // Paddle collisions
        if (isColliding(&ball.pos, &ball.size, &gPaddle1Pos, &gPaddleSize) && ball.vel.x < 0.0f) {
            ball.pos.x = gPaddle1Pos.x + (gPaddleSize.x / 2.0f) + (ball.size.x / 2.0f);
            ball.vel.x *= -1.0f;

            // add angle based on hit location
            float hit = (ball.pos.y - gPaddle1Pos.y) / (gPaddleSize.y / 2.0f);
            ball.vel.y = (BALL_SPEED * 0.65f) * hit;
        }

        if (isColliding(&ball.pos, &ball.size, &gPaddle2Pos, &gPaddleSize) && ball.vel.x > 0.0f) {
            ball.pos.x = gPaddle2Pos.x - (gPaddleSize.x / 2.0f) - (ball.size.x / 2.0f);
            ball.vel.x *= -1.0f;

            float hit = (ball.pos.y - gPaddle2Pos.y) / (gPaddleSize.y / 2.0f);
            ball.vel.y = (BALL_SPEED * 0.65f) * hit;
        }

        // Scoring + reset
        if (ball.pos.x + (ball.size.x / 2.0f) < 0.0f) {
            gScore2++;
            resetBall(ball, +1, i);
        }

        if (ball.pos.x - (ball.size.x / 2.0f) > SCREEN_WIDTH) {
            gScore1++;
            resetBall(ball, -1, i);
        }
    }

    // Game over 
    if (gScore1 >= WIN_SCORE || gScore2 >= WIN_SCORE) {
        gGameOver = true;
    }
}

void render()
{
    BeginDrawing();

    ClearBackground(ColorFromHex(BG_COLOUR));

    // Optional textured background overlay
    if (gBackgroundTexture.id != 0) {
        DrawTexturePro(
            gBackgroundTexture,
            Rectangle{ 0.0f, 0.0f, (float)gBackgroundTexture.width, (float)gBackgroundTexture.height },
            Rectangle{ 0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT },
            Vector2{ 0.0f, 0.0f },
            0.0f,
            WHITE
        );
    }

    Rectangle paddleSrc = {
        0.0f, 0.0f,
        (float)gPaddleTexture.width,
        (float)gPaddleTexture.height
    };

    Rectangle paddle1Dst = {
        gPaddle1Pos.x,
        gPaddle1Pos.y,
        gPaddleSize.x,
        gPaddleSize.y
    };

    Vector2 paddleOrigin = { gPaddleSize.x / 2.0f, gPaddleSize.y / 2.0f };

    DrawTexturePro(
        gPaddleTexture,
        paddleSrc,
        paddle1Dst,
        paddleOrigin,
        0.0f,
        WHITE
    );

    Rectangle paddle2Dst = {
        gPaddle2Pos.x,
        gPaddle2Pos.y,
        gPaddleSize.x,
        gPaddleSize.y
    };

    DrawTexturePro(
        gPaddleTexture,
        paddleSrc,
        paddle2Dst,
        paddleOrigin,
        0.0f,
        WHITE
    );

    
    Rectangle ballSrc = {
        0.0f, 0.0f,
        (float)gBallTexture.width,
        (float)gBallTexture.height
    };

    for (int i = 0; i < MAX_BALLS; i++)
    {
        if (!gBalls[i].active) continue;

        Rectangle ballDst = {
            gBalls[i].pos.x,
            gBalls[i].pos.y,
            gBalls[i].size.x,
            gBalls[i].size.y
        };

        Vector2 ballOrigin = { gBalls[i].size.x / 2.0f, gBalls[i].size.y / 2.0f };

        DrawTexturePro(
            gBallTexture,
            ballSrc,
            ballDst,
            ballOrigin,
            0.0f,
            WHITE
        );
    }


    DrawText(TextFormat("P1: %d", gScore1), 40, 30, 40, WHITE);
    DrawText(TextFormat("P2: %d", gScore2), SCREEN_WIDTH - 170, 30, 40, WHITE);

    DrawText(TextFormat("Mode: %s (T)", gSinglePlayer ? "1P" : "2P"), SCREEN_WIDTH / 2 - 120, 30, 28, WHITE);
    DrawText(TextFormat("Balls: %d (1-3)", gActiveBalls), SCREEN_WIDTH / 2 - 105, 65, 28, WHITE);


    if (gGameOver)
    {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0,0,0,170});

        if (gScore1 > gScore2 && gP1WinTexture.id != 0) {
            DrawTexture(gP1WinTexture,
                SCREEN_WIDTH / 2 - gP1WinTexture.width / 2,
                SCREEN_HEIGHT / 2 - gP1WinTexture.height / 2,
                WHITE
            );
        }
        else if (gScore2 > gScore1 && gP2WinTexture.id != 0) {
            DrawTexture(gP2WinTexture,
                SCREEN_WIDTH / 2 - gP2WinTexture.width / 2,
                SCREEN_HEIGHT / 2 - gP2WinTexture.height / 2,
                WHITE
            );
        }
        DrawText(
            (gScore1 > gScore2) ? "PLAYER 1 WINS!!" : "PLAYER 2 WINS!!",
            SCREEN_WIDTH / 2 - 180,
            SCREEN_HEIGHT / 2 - 40,
            60,
            WHITE
        );

        DrawText("Press R to restart",
                 SCREEN_WIDTH / 2 - 150,
                 SCREEN_HEIGHT / 2 + 40,
                 35,
                 WHITE);
    }

    EndDrawing();
}

void shutdown() 
{
    UnloadTexture(gPaddleTexture);
    UnloadTexture(gBallTexture);
    UnloadTexture(gBackgroundTexture);
    UnloadTexture(gP1WinTexture);
    UnloadTexture(gP2WinTexture);

    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}