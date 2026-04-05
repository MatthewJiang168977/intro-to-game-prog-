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

#ifndef LOSESCENE_H
#define LOSESCENE_H

class LoseScene : public Scene {
public:
    LoseScene();
    LoseScene(Vector2 origin, const char *bgHexCode);
    ~LoseScene();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif