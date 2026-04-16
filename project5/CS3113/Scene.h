#ifndef SCENE_H
#define SCENE_H

#include "Entity.h"

enum SceneType { MENU_SCENE, LEVEL_1, LEVEL_2, LEVEL_3, BATTLE_SCENE, WIN_SCENE, LOSE_SCENE };

class Scene
{
public:
    virtual ~Scene() {}
    virtual void initialise() = 0;
    virtual void processInput() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void shutdown() = 0;
    
    SceneType getNextScene() const { return mNextScene; }
    bool shouldTransition() const { return mShouldTransition; }
    void resetTransition() { mShouldTransition = false; }
    
protected:
    SceneType mNextScene;
    bool mShouldTransition = false;
};

#endif
