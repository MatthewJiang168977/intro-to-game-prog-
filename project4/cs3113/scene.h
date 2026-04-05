/**
* Author: Matthew Jiang
* Assignment: Rise of the AI
* Date due: 04/04/2026, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "Entity.h"

#ifndef SCENE_H
#define SCENE_H

struct GameState
{
    Entity *player;
    Entity *enemies;
    int enemyCount;

    Map *map;

    Music bgm;
    Sound jumpSound;
    Sound hitSound;
    Sound deathSound;
    Sound winSound;

    int nextSceneID;
    int *lives;
};

class Scene
{
protected:
    GameState mGameState;
    Vector2 mOrigin;
    const char *mBGColourHexCode = "#000000";

public:
    Scene();
    Scene(Vector2 origin, const char *bgHexCode);

    virtual void initialise() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void shutdown() = 0;

    GameState   getState()           const { return mGameState;       }
    Vector2     getOrigin()          const { return mOrigin;          }
    const char* getBGColourHexCode() const { return mBGColourHexCode; }

    void setLivesPointer(int *lives) { mGameState.lives = lives; }
};

#endif
