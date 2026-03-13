#include "Entity.h"
#include <math.h>

void Entity::update(float deltaTime)
{
    velocity.x += acceleration.x * deltaTime;
    velocity.y += acceleration.y * deltaTime;

    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;
}

bool Entity::checkCollision(const Entity &other)
{
    float xDistance = fabs(position.x - other.position.x) - ((size.x + other.size.x) / 2.0f);
    float yDistance = fabs(position.y - other.position.y) - ((size.y + other.size.y) / 2.0f);

    return (xDistance < 0.0f && yDistance < 0.0f);
}
