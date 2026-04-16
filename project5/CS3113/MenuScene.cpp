#include "MenuScene.h"

MenuScene::MenuScene() {}
MenuScene::~MenuScene() { shutdown(); }

void MenuScene::initialise() { mBlinkTimer = 0; mShowText = true; mShouldTransition = false; }

void MenuScene::processInput()
{
    if (IsKeyPressed(KEY_ENTER)) { mNextScene = LEVEL_1; mShouldTransition = true; }
}

void MenuScene::update(float deltaTime)
{
    mBlinkTimer += deltaTime;
    if (mBlinkTimer >= 0.5f) { mBlinkTimer = 0; mShowText = !mShowText; }
}

void MenuScene::render()
{
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    
    const char *title = "STACKOVERFLOW";
    DrawText(title, sw/2 - MeasureText(title, 50)/2, 100, 50, WHITE);

    const char *sub = "climb the corporate ladder";
    DrawText(sub, sw/2 - MeasureText(sub, 22)/2, 160, 22, LIGHTGRAY);

    const char *tagline = "a turn-based corporate rogue-like";
    DrawText(tagline, sw/2 - MeasureText(tagline, 16)/2, 190, 16, GRAY);

    int y = 260;
    Color c = {180, 180, 180, 255};
    const char *lines[] = {
        "WASD - Move around the office",
        "ARROW KEYS - Navigate battle menu",
        "ENTER - Select / Confirm",
        "R - Refactor (swap top two abilities)",
        "",
        "Touch an enemy to start a battle!",
        "Manage your CALL STACK to survive.",
    };
    for (int i = 0; i < 7; i++)
    {
        if (lines[i][0] == '\0') { y += 12; continue; }
        DrawText(lines[i], sw/2 - MeasureText(lines[i], 18)/2, y, 18, c);
        y += 26;
    }

    if (mShowText)
    {
        const char *prompt = "Press ENTER to start";
        DrawText(prompt, sw/2 - MeasureText(prompt, 28)/2, sh - 80, 28, YELLOW);
    }
}

void MenuScene::shutdown() {}
