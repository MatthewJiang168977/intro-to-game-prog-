#include "raylib.h"

enum EntityType { PLAYER, PLATFORM, HAZARD, GOAL, MOVING_PLATFORM };

class Entity
{
public:
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
    Vector2 size;

    EntityType type;
    bool active;

    void update(float deltaTime);
    bool checkCollision(const Entity &other);
};
