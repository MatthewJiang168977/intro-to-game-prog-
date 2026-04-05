/**
* Author: Matthew Jiang
* Assignment: Rise of the AI
* Date due: 04/04/2026, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "LoseScene.h"

LoseScene::LoseScene()                                      : Scene { {0.0f}, nullptr   } {}
LoseScene::LoseScene(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

LoseScene::~LoseScene() { shutdown(); }

void LoseScene::initialise()
{
    mGameState.nextSceneID = -1;
    mGameState.player      = nullptr;
    mGameState.map         = nullptr;
    mGameState.enemies     = nullptr;
    mGameState.enemyCount  = 0;
}

void LoseScene::update(float deltaTime)
{
    if (IsKeyPressed(KEY_ENTER)) mGameState.nextSceneID = 0;
}

void LoseScene::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    const char *title = "YOU LOSE";
    int titleFontSize = 60;
    int titleWidth    = MeasureText(title, titleFontSize);

    DrawText(
        title,
        (int) mOrigin.x - titleWidth / 2,
        (int) mOrigin.y - 60,
        titleFontSize,
        RED
    );

    const char *prompt = "Press ENTER to return to menu";
    int promptFontSize = 24;
    int promptWidth    = MeasureText(prompt, promptFontSize);

    DrawText(
        prompt,
        (int) mOrigin.x - promptWidth / 2,
        (int) mOrigin.y + 30,
        promptFontSize,
        LIGHTGRAY
    );
}

void LoseScene::shutdown() { }