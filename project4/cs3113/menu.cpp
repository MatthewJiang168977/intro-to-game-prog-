#include "Menu.h"

Menu::Menu()                                      : Scene { {0.0f}, nullptr   } {}
Menu::Menu(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

Menu::~Menu() { shutdown(); }

void Menu::initialise()
{
    mGameState.nextSceneID = -1;
    mGameState.player      = nullptr;
    mGameState.map         = nullptr;
    mGameState.enemies     = nullptr;
    mGameState.enemyCount  = 0;
}

void Menu::update(float deltaTime)
{
    // Press ENTER to start the game 
    if (IsKeyPressed(KEY_ENTER)) mGameState.nextSceneID = 1;
}

void Menu::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    
    const char *title = "SHADOW REALM";
    int titleFontSize = 60;
    int titleWidth    = MeasureText(title, titleFontSize);

    DrawText(
        title,
        (int) mOrigin.x - titleWidth / 2,
        (int) mOrigin.y - 80,
        titleFontSize,
        WHITE
    );


    const char *prompt = "Press ENTER to start";
    int promptFontSize = 24;
    int promptWidth    = MeasureText(prompt, promptFontSize);

    DrawText(
        prompt,
        (int) mOrigin.x - promptWidth / 2,
        (int) mOrigin.y + 20,
        promptFontSize,
        LIGHTGRAY
    );
}

void Menu::shutdown() { }