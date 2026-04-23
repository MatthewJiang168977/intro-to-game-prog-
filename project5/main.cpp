/**
* Author: Matthew Jiang
* Assignment: STACKOVERFLOW
* Date due: 04/24/2026, 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/ShaderProgram.h"
#include "CS3113/LevelScene.h"
#include "CS3113/MenuScene.h"

//  Constants
// ============================================================
constexpr int SCREEN_WIDTH  = 3200,
              SCREEN_HEIGHT = 2100,
              FPS           = 120;

constexpr Vector2 ORIGIN = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
constexpr float   FIXED_TIMESTEP = 1.0f / 60.0f;

//  Globals
AppStatus gAppStatus     = RUNNING;
float     gPreviousTicks = 0.0f,
          gTimeAccumulator = 0.0f;

Camera2D gCamera = { 0 };

Scene       *gCurrentScene = nullptr;
SceneType    gCurrentType  = MENU_SCENE;
LevelScene  *gLevelScene   = nullptr;
SceneType    gLevelType    = LEVEL_1;

ShaderProgram gShader;
Effects      *gEffects = nullptr;

// Audio
Music gBgMusic[3]  = {0};   // index 0=L1, 1=L2, 2=L3
Sound gAttackSfx   = {0};
Sound gHitSfx      = {0};
int   gCurrentMusicIdx = -1;

// Persistent player state
int     gPlayerHP = 100, gPlayerMaxHP = 100;
Ability gStack[MAX_STACK];
int     gStackSize = 0;
float   gEndScreenTimer = 0;

//  Scene switching
void switchToScene(SceneType type);
void returnFromBattle();

// Stop current music and start a new track (pass -1 to just stop)
void playLevelMusic(int idx)
{
    if (gCurrentMusicIdx >= 0 && gCurrentMusicIdx < 3)
        StopMusicStream(gBgMusic[gCurrentMusicIdx]);
    gCurrentMusicIdx = idx;
    if (idx >= 0 && idx < 3 && gBgMusic[idx].frameCount > 0) {
        PlayMusicStream(gBgMusic[idx]);
        SetMusicVolume(gBgMusic[idx], 0.5f);
    }
}

void switchToScene(SceneType type)
{
    // Save state from level scene before leaving
    if (gLevelScene && gCurrentType != BATTLE_SCENE) {
        gPlayerHP    = gLevelScene->getPlayerHP();
        gPlayerMaxHP = gLevelScene->getPlayerMaxHP();
        gLevelScene->getStack(gStack, &gStackSize);
    }

    // Don't delete level scene when entering battle (it stays alive)
    if (type == BATTLE_SCENE) {
        // Level scene preserved
    } else {
        if (gCurrentScene && gCurrentScene != (Scene*)gLevelScene)
            { gCurrentScene->shutdown(); delete gCurrentScene; }
        if (gLevelScene && type != LEVEL_1 && type != LEVEL_2 && type != LEVEL_3)
            { gLevelScene->shutdown(); delete gLevelScene; gLevelScene = nullptr; }
        gCurrentScene = nullptr;
    }

    gCurrentType = type;

    switch (type)
    {
    case MENU_SCENE:
    {
        gPlayerHP = 100; gPlayerMaxHP = 100; gStackSize = 0;
        playLevelMusic(-1);
        auto *m = new MenuScene();
        m->initialise();
        gCurrentScene = m;
        break;
    }
    case LEVEL_1: case LEVEL_2: case LEVEL_3:
    {
        gLevelType = type;
        gLevelScene = new LevelScene(type);
        gLevelScene->setPlayerHP(gPlayerHP, gPlayerMaxHP);
        if (gStackSize > 0) gLevelScene->setStack(gStack, gStackSize);
        gLevelScene->initialise();
        gCurrentScene = gLevelScene;

        // Start level-appropriate music
        int musicIdx = (int)(type - LEVEL_1); // 0, 1, or 2
        playLevelMusic(musicIdx);

        // Fade in when entering a new level
        gEffects->start(FADEIN);
        break;
    }
    case BATTLE_SCENE:
    {
        // Clear level fade before battle.
        gEffects->setCurrentEffect(NONE);
        gEffects->setAlpha(Effects::TRANSPARENT);

        auto *b = new BattleScene();
        b->setReturnScene(gLevelType);
        b->setSounds(gAttackSfx, gHitSfx);

        Ability stack[MAX_STACK]; int ss;
        gLevelScene->getStack(stack, &ss);
        b->setPlayerData(gLevelScene->getPlayerHP(), gLevelScene->getPlayerMaxHP(),
                         stack, ss, gLevelScene->getPlayerTexture());
        const char *enemyTextures[3] = {
            gLevelScene->getBattleEnemyTexture(),
            nullptr,
            nullptr
        };
        b->setEnemyData(gLevelScene->getBattleEnemyName(),
                        gLevelScene->getBattleEnemyHP(),
                        gLevelScene->getBattleEnemyDamage(),
                        enemyTextures,
                        gLevelScene->getBattleEnemyCount(),
                        gLevelScene->getBattleEnemyIndex());
        b->initialise();
        gCurrentScene = b;
        gLevelScene->clearBattle();
        break;
    }
    case WIN_SCENE: case LOSE_SCENE:
        gEndScreenTimer = 0;
        gEffects->setCurrentEffect(NONE);
        gEffects->setAlpha(Effects::TRANSPARENT);
        gCurrentScene = nullptr;
        playLevelMusic(-1);
        if (gLevelScene) { gLevelScene->shutdown(); delete gLevelScene; gLevelScene = nullptr; }
        break;
    }
}

void returnFromBattle()
{
    BattleScene *b = dynamic_cast<BattleScene*>(gCurrentScene);
    if (!b || !gLevelScene) return;

    BattleResult result = b->getResult();

    if (result == RESULT_WIN) {
        gPlayerHP    = b->getPlayerHP();
        gPlayerMaxHP = b->getPlayerMaxHP();
        Ability stack[MAX_STACK]; int ss;
        b->getStack(stack, &ss);

        gLevelScene->setPlayerHP(gPlayerHP, gPlayerMaxHP);
        gLevelScene->setStack(stack, ss);
        // Remove the full encounter group if one was generated.
        gLevelScene->removeEnemy(-1);
        // Fallback: always remove the lead enemy too.
        gLevelScene->removeEnemy(b->getEnemyIndex());

        b->shutdown(); delete b;
        gCurrentScene = gLevelScene;
        gCurrentType  = gLevelType;

        // Fade in when returning from battle
        gEffects->start(FADEIN);
    }
    else if (result == RESULT_LOSE) {
        b->shutdown(); delete b;
        gCurrentScene = nullptr;
        switchToScene(LOSE_SCENE);
    }
}

//  Core functions
void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "STACKOVERFLOW");
    InitAudioDevice();

    // Load music (looping background tracks per level)
    gBgMusic[0] = LoadMusicStream("assets/level1_bg.mp3");
    gBgMusic[1] = LoadMusicStream("assets/level2_bg.mp3");
    gBgMusic[2] = LoadMusicStream("assets/level3_bg.mp3");
    for (int i = 0; i < 3; i++) {
        if (gBgMusic[i].frameCount > 0)
            gBgMusic[i].looping = true;
    }

    // Load one-shot sound effects
    gAttackSfx = LoadSound("assets/attack.mp3");
    gHitSfx    = LoadSound("assets/hit.mp3");

    // Load shader
    gShader.load("shaders/vertex.glsl", "shaders/fragment.glsl");

    // Effects overlay
    gEffects = new Effects(ORIGIN, (float)SCREEN_WIDTH * 1.5f, (float)SCREEN_HEIGHT * 1.5f);

    // Camera
    gCamera.offset   = ORIGIN;
    gCamera.rotation = 0.0f;
    gCamera.zoom     = 1.0f;

    switchToScene(MENU_SCENE);

    SetTargetFPS(FPS);
}

void processInput()
{
    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) { gAppStatus = TERMINATED; return; }

    if (gCurrentType == WIN_SCENE || gCurrentType == LOSE_SCENE) {
        if (IsKeyPressed(KEY_ENTER)) switchToScene(MENU_SCENE);
        return;
    }

    if (gCurrentScene) gCurrentScene->processInput();
}

void update()
{
    float ticks = (float)GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    // Keep the active music stream buffered
    if (gCurrentMusicIdx >= 0 && gCurrentMusicIdx < 3)
        UpdateMusicStream(gBgMusic[gCurrentMusicIdx]);

    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        if (gCurrentType == WIN_SCENE || gCurrentType == LOSE_SCENE)
        {
            gEndScreenTimer += FIXED_TIMESTEP;
        }
        else if (gCurrentScene)
        {
            gCurrentScene->update(FIXED_TIMESTEP);

            // Update effects every frame.
            Vector2 effectTarget = ORIGIN;
            gEffects->update(FIXED_TIMESTEP, &effectTarget);

            // Check battle trigger
            if (gCurrentType >= LEVEL_1 && gCurrentType <= LEVEL_3 && gLevelScene) {
                if (gLevelScene->wantsBattle()) {
                    switchToScene(BATTLE_SCENE);
                    break;
                }
            }

            // Check scene transitions
            if (gCurrentScene && gCurrentScene->shouldTransition()) {
                SceneType next = gCurrentScene->getNextScene();
                if (gCurrentType == BATTLE_SCENE)
                    returnFromBattle();
                else
                    switchToScene(next);
                break;
            }
        }

        deltaTime -= FIXED_TIMESTEP;
    }

    gTimeAccumulator = deltaTime;
}

void render()
{
    BeginDrawing();
    ClearBackground((Color){15, 15, 25, 255});

    if (gCurrentType == WIN_SCENE)
    {
        int sw = GetScreenWidth();
        const char *t = "YOU LEAKED THE IPO";
        DrawText(t, sw/2 - MeasureText(t, 48)/2, 180, 48, GREEN);
        const char *s = "MegaCorp has been exposed. The people win.";
        DrawText(s, sw/2 - MeasureText(s, 20)/2, 260, 20, LIGHTGRAY);
        if (gEndScreenTimer > 1) {
            const char *p = "Press ENTER to return to menu";
            DrawText(p, sw/2 - MeasureText(p, 20)/2, 380, 20, YELLOW);
        }
    }
    else if (gCurrentType == LOSE_SCENE)
    {
        int sw = GetScreenWidth();
        const char *t = "YOU'VE BEEN FIRED";
        DrawText(t, sw/2 - MeasureText(t, 48)/2, 180, 48, RED);
        const char *s = "Security has escorted you from the building.";
        DrawText(s, sw/2 - MeasureText(s, 20)/2, 260, 20, LIGHTGRAY);
        if (gEndScreenTimer > 1) {
            const char *p = "Press ENTER to try again";
            DrawText(p, sw/2 - MeasureText(p, 20)/2, 380, 20, YELLOW);
        }
    }
    else if (gCurrentScene)
    {
        // Apply burnout shader during exploration (not during battle or menu)
        bool useShader = (gCurrentType >= LEVEL_1 && gCurrentType <= LEVEL_3) 
                         && gShader.isLoaded();

        if (useShader && gLevelScene) {
            float hpRatio = gLevelScene->getPlayerHPRatio();
            gShader.setFloat("hp_ratio", hpRatio);
            float levelDarkness = 0.08f;
            if (gCurrentType == LEVEL_2) levelDarkness = 0.16f;
            else if (gCurrentType == LEVEL_3) levelDarkness = 0.24f;
            gShader.setFloat("level_darkness", levelDarkness);
            gShader.begin();
        }

        gCurrentScene->render();

        if (useShader) gShader.end();

        // Draw key prompt after shader pass.
        if (gCurrentType >= LEVEL_1 && gCurrentType <= LEVEL_3
            && gLevelScene && gLevelScene->shouldShowPickupPrompt()) {
            int tx = GetScreenWidth()/2 - 120;
            int ty = GetScreenHeight() - 48;
            DrawRectangle(tx - 12, ty - 6, 250, 30, (Color){0, 0, 0, 200});
            DrawText("[E] Pick up ability", tx, ty, 20, YELLOW);
        }

        // Render effects overlay (fade in/out)
        gEffects->render();
    }
    else
    {
        const char *msg = "Scene unavailable. Press ENTER to return to menu.";
        DrawText(msg, GetScreenWidth()/2 - MeasureText(msg, 24)/2,
                 GetScreenHeight()/2 - 12, 24, ORANGE);
    }

    EndDrawing();
}

void shutdown()
{
    if (gCurrentScene && gCurrentScene != (Scene*)gLevelScene)
        { gCurrentScene->shutdown(); delete gCurrentScene; }
    if (gLevelScene) { gLevelScene->shutdown(); delete gLevelScene; }

    delete gEffects;
    gEffects = nullptr;

    gShader.unload();

    // Unload audio
    for (int i = 0; i < 3; i++)
        if (gBgMusic[i].frameCount > 0) UnloadMusicStream(gBgMusic[i]);
    if (gAttackSfx.frameCount > 0) UnloadSound(gAttackSfx);
    if (gHitSfx.frameCount > 0)    UnloadSound(gHitSfx);

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
        render();
    }

    shutdown();
    return 0;
}
