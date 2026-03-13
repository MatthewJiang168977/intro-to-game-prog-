/**
* Author: Matthew Jiang
* Assignment: Lunar Lander
* Date due: 3/14/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include "Entity.h"

// Enums
enum AppStatus { TERMINATED, RUNNING };
enum GameState { PLAYING, MISSION_FAILED, MISSION_ACCOMPLISHED };

// Global Constants
constexpr int SCREEN_WIDTH  = 1800;
constexpr int SCREEN_HEIGHT = 1100;
constexpr int FPS           = 120;

constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;
constexpr int   MAX_STEPS      = 6;

constexpr float GRAVITY            = 300.0f;
constexpr float HORIZONTAL_THRUST  = 900.0f;
constexpr float UPWARD_THRUST      = 1200.0f;
constexpr float DRAG               = 0.95f;
constexpr float STARTING_FUEL      = 200.0f;
constexpr float FUEL_COST_PER_STEP = 0.6f;
Texture2D gFlameTexture;
Texture2D gShipTexture;
int gFlameFrame = 0;
float gFlameTimer = 0.0f;
const int FLAME_COLUMNS = 7;
const int FLAME_ROWS = 3; 
const int FLAME_FRAMES = 21;
const float FLAME_FRAME_TIME = 0.08f;
bool gThrusting = false;
const int STAR_COUNT = 80;
Vector2 gStars[STAR_COUNT];
float gStarSizes[STAR_COUNT]; 

// Global state
bool gStranded = false;

AppStatus gAppStatus = RUNNING;
GameState gGameState = PLAYING;

float gPreviousTicks = 0.0f;
float gAccumulator   = 0.0f;
float gFuel          = STARTING_FUEL;

Entity gPlayer;
Entity gPlatforms[5];

// Function declarations
void initialise();
void restartGame();
void processInput();
void update();
void fixedUpdate(float deltaTime);
void render();
void shutdown();

float clampf(float value, float minValue, float maxValue);
bool landedSafely(const Entity &player, const Entity &goal);

float clampf(float value, float minValue, float maxValue)
{
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

bool landedSafely(const Entity &player, const Entity &goal)
{
    bool slowX = fabs(player.velocity.x) < 250.0f;
    bool slowY = fabs(player.velocity.y) < 250.0f;

    float playerBottom = player.position.y + player.size.y / 2.0f;
    float goalTop      = goal.position.y - goal.size.y / 2.0f;
    bool nearTop       = fabs(playerBottom - goalTop) < 40.0f;

    return slowX && slowY && nearTop;
}

void restartGame()
{
    gGameState = PLAYING;
    gFuel      = STARTING_FUEL;
    gStranded  = false;

    gPlayer.position     = { 180.0f, 120.0f };
    gPlayer.velocity     = { 0.0f, 0.0f };
    gPlayer.acceleration = { 0.0f, 0.0f };
    gPlayer.size         = { 150.0f, 150.0f };
    gPlayer.type         = PLAYER;
    gPlayer.active       = true;

    // Regular platform
    gPlatforms[0].position     = { 420.0f, 900.0f };
    gPlatforms[0].velocity     = { 0.0f, 0.0f };
    gPlatforms[0].acceleration = { 0.0f, 0.0f };
    gPlatforms[0].size         = { 300.0f, 35.0f };
    gPlatforms[0].type         = PLATFORM;
    gPlatforms[0].active       = true;

    // Hazard
    gPlatforms[1].position     = { 900.0f, 980.0f };
    gPlatforms[1].velocity     = { 0.0f, 0.0f };
    gPlatforms[1].acceleration = { 0.0f, 0.0f };
    gPlatforms[1].size         = { 450.0f, 80.0f };
    gPlatforms[1].type         = HAZARD;
    gPlatforms[1].active       = true;

    // Goal platform
    gPlatforms[2].position     = { 1520.0f, 900.0f };
    gPlatforms[2].velocity     = { 0.0f, 0.0f };
    gPlatforms[2].acceleration = { 0.0f, 0.0f };
    gPlatforms[2].size         = { 200.0f, 25.0f };
    gPlatforms[2].type         = GOAL;
    gPlatforms[2].active       = true;

    // Moving platform
    gPlatforms[3].position     = { 850.0f, 700.0f };
    gPlatforms[3].velocity     = { 220.0f, 0.0f };
    gPlatforms[3].acceleration = { 0.0f, 0.0f };
    gPlatforms[3].size         = { 220.0f, 30.0f };
    gPlatforms[3].type         = MOVING_PLATFORM;
    gPlatforms[3].active       = true;

    // Another regular platform
    gPlatforms[4].position     = { 1200.0f, 560.0f };
    gPlatforms[4].velocity     = { 0.0f, 0.0f };
    gPlatforms[4].acceleration = { 0.0f, 0.0f };
    gPlatforms[4].size         = { 180.0f, 30.0f };
    gPlatforms[4].type         = PLATFORM;
    gPlatforms[4].active       = true;

    gPreviousTicks = (float)GetTime();
    gAccumulator   = 0.0f;
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lunar Lander");
    gFlameTexture = LoadTexture("assets/exhaust_01_spritesheet.png");
    gShipTexture = LoadTexture("assets/ericship.png");
    for (int i = 0; i < STAR_COUNT; i++) {
        gStars[i].x = (float)(rand() % SCREEN_WIDTH);
        gStars[i].y = (float)(rand() % SCREEN_HEIGHT);
        gStarSizes[i] = (float)((rand() % 3) + 1);
    }
    SetTargetFPS(FPS);
    restartGame();
}

void processInput()
{
    if (WindowShouldClose()) {
        gAppStatus = TERMINATED;
        return;
    }

    gThrusting = false; 

    if (gGameState != PLAYING || gStranded)
    {
        if (IsKeyPressed(KEY_R)) {
            restartGame();
        }
        return;
    }

    // Gravity always acts downward
    gPlayer.acceleration.x = 0.0f;
    gPlayer.acceleration.y = GRAVITY;

    if (gFuel <= 0.0f) return;

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
    {
        gPlayer.acceleration.x = -HORIZONTAL_THRUST;
        gFuel -= FUEL_COST_PER_STEP;
    }

    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
    {
        gPlayer.acceleration.x = HORIZONTAL_THRUST;
        gFuel -= FUEL_COST_PER_STEP;
    }

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_SPACE))
    {
        gPlayer.acceleration.y = -UPWARD_THRUST;
        gFuel -= FUEL_COST_PER_STEP;
        gThrusting = true; 
    }

    gFuel = clampf(gFuel, 0.0f, STARTING_FUEL);
}

void fixedUpdate(float deltaTime)
{
    gStranded = false;

    if (gGameState != PLAYING) return;

    // Moving platform movement
    gPlatforms[3].position.x += gPlatforms[3].velocity.x * deltaTime;

    if (gPlatforms[3].position.x > 1150.0f) {
        gPlatforms[3].position.x = 1150.0f;
        gPlatforms[3].velocity.x *= -1.0f;
    }
    if (gPlatforms[3].position.x < 650.0f) {
        gPlatforms[3].position.x = 650.0f;
        gPlatforms[3].velocity.x *= -1.0f;
    }

    // Player physics through Entity class
    gPlayer.update(deltaTime);

    // Drift / gradual slowdown when no horizontal thrust
    if (fabs(gPlayer.acceleration.x) < 0.001f) {
        gPlayer.velocity.x *= DRAG;
    }

    // World bounds
    float halfW = gPlayer.size.x / 2.0f;
    float halfH = gPlayer.size.y / 2.0f;

    if (gPlayer.position.x - halfW <= 0.0f || gPlayer.position.x + halfW >= SCREEN_WIDTH) {
        gGameState = MISSION_FAILED;
        return;
    }

    if (gPlayer.position.y + halfH >= SCREEN_HEIGHT) {
        gGameState = MISSION_FAILED;
        return;
    }

    if (gPlayer.position.y - halfH <= 0.0f) {
        gPlayer.position.y = halfH;
        gPlayer.velocity.y = 0.0f;
    }

    // Collisions
    for (int i = 0; i < 5; i++)
    {
        if (!gPlayer.checkCollision(gPlatforms[i])) continue;

        if (gPlatforms[i].type == HAZARD) {
            gGameState = MISSION_FAILED;
            return;
        }

        if (gPlatforms[i].type == GOAL)
        {
            float playerBottom = gPlayer.position.y + gPlayer.size.y / 2.0f;
            float goalTop      = gPlatforms[i].position.y - gPlatforms[i].size.y / 2.0f;

            bool landingFromAbove = (gPlayer.velocity.y >= -20.0f && playerBottom >= goalTop - 35.0f);

            if (landingFromAbove)
            {
                if (landedSafely(gPlayer, gPlatforms[i])) {
                    gPlayer.position.y = goalTop - gPlayer.size.y / 2.0f;
                    gPlayer.velocity.y = 0.0f;
                    gGameState = MISSION_ACCOMPLISHED;
                } else {
                    gGameState = MISSION_FAILED;
                }
            }

            return;
        }

        if (gPlatforms[i].type == PLATFORM || gPlatforms[i].type == MOVING_PLATFORM)
        {
            float playerBottom = gPlayer.position.y + gPlayer.size.y / 2.0f;
            float platformTop  = gPlatforms[i].position.y - gPlatforms[i].size.y / 2.0f;

            if (gPlayer.velocity.y >= 0.0f && playerBottom >= platformTop - 18.0f)
            {
                gPlayer.position.y = platformTop - gPlayer.size.y / 2.0f;
                gPlayer.velocity.y = 0.0f;

                if (gPlatforms[i].type == MOVING_PLATFORM) {
                    gPlayer.position.x += gPlatforms[i].velocity.x * deltaTime;
                }

                if (gFuel <= 0.0f && fabs(gPlayer.velocity.x) < 15.0f) {
                    gStranded = true;
                }
            }
        }
    }
    if (gThrusting){
        gFlameTimer += deltaTime;

        if (gFlameTimer >= FLAME_FRAME_TIME)
        {
            gFlameFrame++;
            gFlameFrame %= FLAME_FRAMES;
            gFlameTimer = 0.0f;
        }
    }
    else {
        gFlameFrame = 0;
        gFlameTimer = 0.0f;
    }
}

void update()
{
    float ticks = (float)GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    deltaTime += gAccumulator;

    if (deltaTime > FIXED_TIMESTEP * MAX_STEPS) {
        deltaTime = FIXED_TIMESTEP * MAX_STEPS;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        fixedUpdate(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;
    }

    gAccumulator = deltaTime;
}

void render()
{
    BeginDrawing();

    ClearBackground(Color{ 5, 5, 20, 255 });

    //draw stars
    for (int i = 0; i < STAR_COUNT; i++) {
        DrawCircleV(gStars[i], gStarSizes[i], WHITE);
    }

    // Draw player
    DrawTexturePro(
        gShipTexture,
        Rectangle{0, 0, (float)gShipTexture.width, (float)gShipTexture.height},
        Rectangle{gPlayer.position.x, gPlayer.position.y, gPlayer.size.x, gPlayer.size.y},
        Vector2{gPlayer.size.x / 2.0f, gPlayer.size.y / 2.0f},
        0.0f,
        WHITE
    );

    // Draw platforms
    for (int i = 0; i < 5; i++)
    {
        Color color = GRAY;

        if (gPlatforms[i].type == HAZARD)          color = RED;
        if (gPlatforms[i].type == GOAL)            color = GREEN;
        if (gPlatforms[i].type == MOVING_PLATFORM) color = SKYBLUE;

        DrawRectanglePro(
            Rectangle{
                gPlatforms[i].position.x,
                gPlatforms[i].position.y,
                gPlatforms[i].size.x,
                gPlatforms[i].size.y
            },
            Vector2{ gPlatforms[i].size.x / 2.0f, gPlatforms[i].size.y / 2.0f },
            0.0f,
            color
        );
    }

    // UI
    DrawText(TextFormat("Fuel: %d", (int)gFuel), 40, 30, 40, WHITE);
    DrawText(TextFormat("Vel X: %.1f", gPlayer.velocity.x), 40, 80, 28, WHITE);
    DrawText(TextFormat("Vel Y: %.1f", gPlayer.velocity.y), 40, 115, 28, WHITE);
    DrawText("A / D or Arrow Keys = left/right thrust", 40, SCREEN_HEIGHT - 80, 28, WHITE);
    DrawText("W / UP / SPACE = upward thrust", 40, SCREEN_HEIGHT - 45, 28, WHITE);

    if (gGameState == MISSION_FAILED) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{ 0, 0, 0, 170 });
        DrawText("MISSION FAILED :(", SCREEN_WIDTH / 2 - 220, SCREEN_HEIGHT / 2 - 20, 60, WHITE);
        DrawText("Press R to restart ", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 + 60, 35, WHITE);
    }
    else if (gGameState == MISSION_ACCOMPLISHED) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{ 0, 0, 0, 170 });
        DrawText("MISSION ACCOMPLISHED :)", SCREEN_WIDTH / 2 - 330, SCREEN_HEIGHT / 2 - 20, 60, WHITE);
        DrawText("Press R to restart", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 + 60, 35, WHITE);
    }

    if (gStranded && gGameState == PLAYING)
    {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{ 0, 0, 0, 140 });
        DrawText("OUT OF FUEL: PRESS R TO RESTART :(", SCREEN_WIDTH / 2 - 330, SCREEN_HEIGHT / 2 - 20, 50, WHITE);
    }

    if (gThrusting) {
        float frameWidth  = (float)gFlameTexture.width / FLAME_COLUMNS;
        float frameHeight = (float)gFlameTexture.height / FLAME_ROWS;

        int frameX = gFlameFrame % FLAME_COLUMNS;
        int frameY = gFlameFrame / FLAME_COLUMNS;

        Rectangle source = {
            frameWidth * frameX,
            frameHeight * frameY,
            frameWidth,
            frameHeight
        };

        Rectangle dest = {
            gPlayer.position.x,
            gPlayer.position.y + gPlayer.size.y / 2.0f + 70.0f,
            frameWidth * 1.5f,
            frameHeight * 1.5f
        };

        Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f };

        DrawTexturePro(gFlameTexture, source, dest, origin, 0.0f, WHITE);
    }

    EndDrawing();
}

void shutdown()
{
    UnloadTexture(gFlameTexture);
    UnloadTexture(gShipTexture);
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
