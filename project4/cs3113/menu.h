#include "LevelC.h"

#ifndef MENU_H
#define MENU_H

class Menu : public Scene {
public:
    Menu();
    Menu(Vector2 origin, const char *bgHexCode);
    ~Menu();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif