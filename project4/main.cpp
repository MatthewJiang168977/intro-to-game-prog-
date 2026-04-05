/**
* Author: Matthew Jiang
* Assignment: Rise of the AI
* Date due: 04/04/2026, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/Menu.h"
#include "CS3113/WinScene.h"
#include "CS3113/LoseScene.h"

// Global Constants
constexpr int SCREEN_WIDTH     = 1000,
              SCREEN_HEIGHT    = 600,
              FPS              = 120,
              NUMBER_OF_SCENES = 6,
              INITIAL_LIVES    = 3;

constexpr Vector2 ORIGIN = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;

Camera2D gCamera = { 0 };
int gLives       = INITIAL_LIVES;

Scene *gCurrentScene = nullptr;
std::vector<Scene*> gScenes = {};

Menu      *gMenu      = nullptr;
LevelA    *gLevelA    = nullptr;
LevelB    *gLevelB    = nullptr;
LevelC    *gLevelC    = nullptr;
WinScene  *gWinScene  = nullptr;
LoseScene *gLoseScene = nullptr;

// Function Declarations
void switchToScene(Scene *scene);
void initialise();
void processInput();
void update();
void render();
void shutdown();

void switchToScene(Scene *scene)
{
    gCurrentScene = scene;
    gCurrentScene->initialise();
    gCurrentScene->setLivesPointer(&gLives);

    // Reset camera 
    if (gCurrentScene->getState().player != nullptr)
        gCamera.target = gCurrentScene->getState().player->getPosition();
    else
        gCamera.target = ORIGIN;

    // If returning to Menu from Win/Lose, reset lives
    if (scene == gMenu) gLives = INITIAL_LIVES;
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Rise of the AI");
    InitAudioDevice();

    // 0=Menu, 1=LevelA, 2=LevelB, 3=LevelC, 4=Win, 5=Lose
    gMenu      = new Menu(ORIGIN, "#1A1A2E");
    gLevelA    = new LevelA(ORIGIN, "#2D572C");
    gLevelB    = new LevelB(ORIGIN, "#011627");
    gLevelC    = new LevelC(ORIGIN, "#3C1518");
    gWinScene  = new WinScene(ORIGIN, "#0D1B2A");
    gLoseScene = new LoseScene(ORIGIN, "#0D1B2A");

    gScenes.push_back(gMenu);      // 0
    gScenes.push_back(gLevelA);    // 1
    gScenes.push_back(gLevelB);    // 2
    gScenes.push_back(gLevelC);    // 3
    gScenes.push_back(gWinScene);  // 4
    gScenes.push_back(gLoseScene); // 5

    switchToScene(gScenes[0]);

    gCamera.offset   = ORIGIN;
    gCamera.rotation = 0.0f;
    gCamera.zoom     = 1.0f;

    SetTargetFPS(FPS);
}

void processInput()
{
    if (gCurrentScene->getState().player != nullptr)
    {
        gCurrentScene->getState().player->resetMovement();

        if      (IsKeyDown(KEY_A)) gCurrentScene->getState().player->moveLeft();
        else if (IsKeyDown(KEY_D)) gCurrentScene->getState().player->moveRight();

        if (IsKeyPressed(KEY_W) &&
            gCurrentScene->getState().player->isCollidingBottom())
        {
            gCurrentScene->getState().player->jump();
            PlaySound(gCurrentScene->getState().jumpSound);
        }

        if (GetLength(gCurrentScene->getState().player->getMovement()) > 1.0f)
            gCurrentScene->getState().player->normaliseMovement();
    }

    // Switch levels with number keys 
    if (IsKeyPressed(KEY_ONE))   switchToScene(gScenes[1]);
    if (IsKeyPressed(KEY_TWO))   switchToScene(gScenes[2]);
    if (IsKeyPressed(KEY_THREE)) switchToScene(gScenes[3]);

    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
}

void update()
{
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        gCurrentScene->update(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;

        // Pan camera to follow player
        if (gCurrentScene->getState().player != nullptr)
        {
            Vector2 currentPlayerPosition = {
                gCurrentScene->getState().player->getPosition().x,
                ORIGIN.y
            };
            panCamera(&gCamera, &currentPlayerPosition);
        }
    }
}

void render()
{
    BeginDrawing();
    BeginMode2D(gCamera);

    gCurrentScene->render();

    EndMode2D();

    // HUD
    if (gCurrentScene->getState().player != nullptr)
    {
        char livesText[32];
        snprintf(livesText, sizeof(livesText), "Lives: %d", gLives);
        DrawText(livesText, 20, 20, 30, WHITE);
    }

    EndDrawing();
}

void shutdown()
{
    delete gMenu;
    delete gLevelA;
    delete gLevelB;
    delete gLevelC;
    delete gWinScene;
    delete gLoseScene;

    for (int i = 0; i < NUMBER_OF_SCENES; i++) gScenes[i] = nullptr;

    CloseAudioDevice();
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();

        if (gCurrentScene->getState().nextSceneID >= 0)
        {
            int id = gCurrentScene->getState().nextSceneID;
            switchToScene(gScenes[id]);
        }

        render();
    }

    shutdown();

    return 0;
}