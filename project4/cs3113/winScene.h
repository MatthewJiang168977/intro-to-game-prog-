/**
* Author: Matthew Jiang
* Assignment: Rise of the AI
* Date due: 04/04/2026, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "LevelC.h"

#ifndef WINSCENE_H
#define WINSCENE_H

class WinScene : public Scene {
public:
    WinScene();
    WinScene(Vector2 origin, const char *bgHexCode);
    ~WinScene();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif