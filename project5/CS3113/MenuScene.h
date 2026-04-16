#ifndef MENU_SCENE_H
#define MENU_SCENE_H
#include "Scene.h"

class MenuScene : public Scene
{
public:
    MenuScene();
    ~MenuScene();
    void initialise() override;
    void processInput() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
private:
    float mBlinkTimer = 0.0f;
    bool  mShowText = true;
};

#endif
