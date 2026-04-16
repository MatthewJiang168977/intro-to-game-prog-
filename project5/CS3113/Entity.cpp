#include "Entity.h"

Entity::Entity() : mPosition{0,0}, mMovement{0,0}, mVelocity{0,0}, mAcceleration{0,0},
    mScale{DEFAULT_SIZE, DEFAULT_SIZE}, mColliderDimensions{DEFAULT_SIZE, DEFAULT_SIZE},
    mTexture{0}, mTextureType{SINGLE}, mAngle{0}, mSpriteSheetDimensions{},
    mDirection{DOWN}, mAnimationAtlas{{}}, mAnimationIndices{}, mFrameSpeed{0},
    mSpeed{DEFAULT_SPEED}, mEntityType{ENTITY_NONE}, mAIType{WANDERER}, mAIState{IDLE},
    mHP{0}, mMaxHP{0} {}

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
    EntityType entityType) : mPosition{position}, mVelocity{0,0}, mAcceleration{0,0},
    mScale{scale}, mMovement{0,0}, mColliderDimensions{scale},
    mTexture{LoadTexture(textureFilepath)}, mTextureType{SINGLE}, mDirection{DOWN},
    mAnimationAtlas{{}}, mAnimationIndices{}, mFrameSpeed{0}, mSpeed{DEFAULT_SPEED},
    mAngle{0}, mEntityType{entityType}, mAIType{WANDERER}, mAIState{IDLE}, mHP{0}, mMaxHP{0} {}

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
    TextureType textureType, Vector2 spriteSheetDimensions, 
    std::map<Direction, std::vector<int>> animationAtlas, EntityType entityType) : 
    mPosition{position}, mVelocity{0,0}, mAcceleration{0,0}, mMovement{0,0}, mScale{scale},
    mColliderDimensions{scale}, mTexture{LoadTexture(textureFilepath)}, mTextureType{ATLAS},
    mSpriteSheetDimensions{spriteSheetDimensions}, mAnimationAtlas{animationAtlas},
    mDirection{DOWN},
    mAnimationIndices{animationAtlas.count(DOWN) ? animationAtlas.at(DOWN) : std::vector<int>{}},
    mFrameSpeed{DEFAULT_FRAME_SPEED}, mAngle{0}, mSpeed{DEFAULT_SPEED},
    mEntityType{entityType}, mAIType{WANDERER}, mAIState{IDLE}, mHP{0}, mMaxHP{0} {}

Entity::~Entity() { UnloadTexture(mTexture); }

void Entity::takeDamage(int amount)
{
    mHP -= amount;
    if (mHP > mMaxHP) mHP = mMaxHP;
    if (amount > 0) mFlashTimer = 0.15f;
    if (mHP <= 0) { mHP = 0; deactivate(); }
}

// ---- Collision ----
void Entity::checkCollisionY(Entity *e, int count)
{
    for (int i = 0; i < count; i++) {
        Entity *o = &e[i];
        if (isColliding(o)) {
            float yDist = fabs(mPosition.y - o->mPosition.y);
            float yOvlp = fabs(yDist - (mColliderDimensions.y/2) - (o->mColliderDimensions.y/2));
            if (mVelocity.y > 0)      { mPosition.y -= yOvlp; mVelocity.y = 0; mIsCollidingBottom = true; }
            else if (mVelocity.y < 0)  { mPosition.y += yOvlp; mVelocity.y = 0; mIsCollidingTop = true; }
        }
    }
}

void Entity::checkCollisionX(Entity *e, int count)
{
    for (int i = 0; i < count; i++) {
        Entity *o = &e[i];
        if (isColliding(o)) {
            float yDist = fabs(mPosition.y - o->mPosition.y);
            float yOvlp = fabs(yDist - (mColliderDimensions.y/2) - (o->mColliderDimensions.y/2));
            if (yOvlp < Y_COLLISION_THRESHOLD) continue;
            float xDist = fabs(mPosition.x - o->mPosition.x);
            float xOvlp = fabs(xDist - (mColliderDimensions.x/2) - (o->mColliderDimensions.x/2));
            if (mVelocity.x > 0)      { mPosition.x -= xOvlp; mVelocity.x = 0; mIsCollidingRight = true; }
            else if (mVelocity.x < 0)  { mPosition.x += xOvlp; mVelocity.x = 0; mIsCollidingLeft = true; }
        }
    }
}

void Entity::checkCollisionY(Map *map)
{
    if (!map) return;
    Vector2 probes[] = {
        {mPosition.x, mPosition.y - mColliderDimensions.y/2},
        {mPosition.x - mColliderDimensions.x/2, mPosition.y - mColliderDimensions.y/2},
        {mPosition.x + mColliderDimensions.x/2, mPosition.y - mColliderDimensions.y/2},
        {mPosition.x, mPosition.y + mColliderDimensions.y/2},
        {mPosition.x - mColliderDimensions.x/2, mPosition.y + mColliderDimensions.y/2},
        {mPosition.x + mColliderDimensions.x/2, mPosition.y + mColliderDimensions.y/2},
    };
    float xo, yo;
    // Top probes
    for (int i = 0; i < 3; i++) {
        if (map->isSolidTileAt(probes[i], &xo, &yo) && mVelocity.y < 0) {
            mPosition.y += yo * 1.01f; mVelocity.y = 0; mIsCollidingTop = true; break;
        }
    }
    // Bottom probes
    for (int i = 3; i < 6; i++) {
        if (map->isSolidTileAt(probes[i], &xo, &yo) && mVelocity.y > 0) {
            mPosition.y -= yo * 1.01f; mVelocity.y = 0; mIsCollidingBottom = true; break;
        }
    }
}

void Entity::checkCollisionX(Map *map)
{
    if (!map) return;
    float xo, yo;
    Vector2 rProbes[] = {
        {mPosition.x + mColliderDimensions.x/2, mPosition.y},
        {mPosition.x + mColliderDimensions.x/2, mPosition.y - mColliderDimensions.y/2},
        {mPosition.x + mColliderDimensions.x/2, mPosition.y + mColliderDimensions.y/2},
    };
    Vector2 lProbes[] = {
        {mPosition.x - mColliderDimensions.x/2, mPosition.y},
        {mPosition.x - mColliderDimensions.x/2, mPosition.y - mColliderDimensions.y/2},
        {mPosition.x - mColliderDimensions.x/2, mPosition.y + mColliderDimensions.y/2},
    };
    for (int i = 0; i < 3; i++) {
        if (map->isSolidTileAt(rProbes[i], &xo, &yo) && mVelocity.x > 0) {
            mPosition.x -= xo * 1.01f; mVelocity.x = 0; mIsCollidingRight = true; break;
        }
    }
    for (int i = 0; i < 3; i++) {
        if (map->isSolidTileAt(lProbes[i], &xo, &yo) && mVelocity.x < 0) {
            mPosition.x += xo * 1.01f; mVelocity.x = 0; mIsCollidingLeft = true; break;
        }
    }
}

bool Entity::isColliding(Entity *other) const
{
    if (!other->isActive() || other == this) return false;
    float xd = fabs(mPosition.x - other->mPosition.x) - ((mColliderDimensions.x + other->getColliderDimensions().x)/2);
    float yd = fabs(mPosition.y - other->mPosition.y) - ((mColliderDimensions.y + other->getColliderDimensions().y)/2);
    return (xd < 0 && yd < 0);
}

void Entity::animate(float deltaTime)
{
    if (mAnimationAtlas.empty() || !mAnimationAtlas.count(mDirection)) return;
    mAnimationIndices = mAnimationAtlas.at(mDirection);
    if (mAnimationIndices.empty()) return;
    mAnimationTime += deltaTime;
    float fps = 1.0f / mFrameSpeed;
    if (mAnimationTime >= fps) {
        mAnimationTime = 0;
        mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mAnimationIndices.size();
    }
}

// ---- AI ----
void Entity::AIWander()
{
    int r = GetRandomValue(0, 100);
    if (r < 2) {
        switch (GetRandomValue(0, 3)) {
            case 0: moveLeft(); break;  case 1: moveRight(); break;
            case 2: moveUp(); break;    case 3: moveDown(); break;
        }
    }
}

void Entity::AIFollow(Entity *target)
{
    if (!target || !target->isActive()) return;
    float dist = Vector2Distance(mPosition, target->getPosition());
    if (mAIState == IDLE) { if (dist < 300) mAIState = FOLLOWING; }
    else if (mAIState == FOLLOWING) {
        float dx = target->getPosition().x - mPosition.x;
        float dy = target->getPosition().y - mPosition.y;
        if (fabs(dx) > fabs(dy)) { if (dx > 0) moveRight(); else moveLeft(); }
        else { if (dy > 0) moveDown(); else moveUp(); }
        if (dist > 400) mAIState = IDLE;
    }
}

void Entity::AIPatrol()
{
    if (mWaypoints.empty()) return;
    Vector2 t = mWaypoints[mCurrentWaypoint];
    if (Vector2Distance(mPosition, t) < 10)
        mCurrentWaypoint = (mCurrentWaypoint + 1) % mWaypoints.size();
    t = mWaypoints[mCurrentWaypoint];
    float dx = t.x - mPosition.x, dy = t.y - mPosition.y;
    if (fabs(dx) > fabs(dy)) { if (dx > 0) moveRight(); else moveLeft(); }
    else { if (dy > 0) moveDown(); else moveUp(); }
}

void Entity::AIDisrupt(Entity *target)
{
    if (!target || !target->isActive()) return;
    float dist = Vector2Distance(mPosition, target->getPosition());
    if (mAIState == IDLE) { AIWander(); if (dist < 250) mAIState = DASHING; }
    else if (mAIState == DASHING) {
        float dx = target->getPosition().x - mPosition.x;
        float dy = target->getPosition().y - mPosition.y;
        if (fabs(dx) > fabs(dy)) { if (dx > 0) moveRight(); else moveLeft(); }
        else { if (dy > 0) moveDown(); else moveUp(); }
        if (dist < 30 || dist > 400) { mAIState = COOLDOWN; mCooldownTimer = mCooldownDuration; }
    }
}

void Entity::AIActivate(Entity *target)
{
    switch (mAIType) {
        case WANDERER:  AIWander(); break;
        case FOLLOWER:  AIFollow(target); break;
        case PATROLLER: AIPatrol(); break;
        case DISRUPTOR: AIDisrupt(target); break;
    }
}

// ---- Update ----
void Entity::update(float deltaTime, Entity *player, Map *map,
    Entity *collidableEntities, int collisionCheckCount)
{
    if (mEntityStatus == INACTIVE) return;
    if (mFlashTimer > 0) mFlashTimer -= deltaTime;
    if (mAIState == COOLDOWN) { mCooldownTimer -= deltaTime; if (mCooldownTimer <= 0) mAIState = IDLE; }
    if (mLifetime > 0) { mLifetime -= deltaTime; if (mLifetime <= 0) { deactivate(); return; } }
    if (mEntityType == NPC) AIActivate(player);
    resetColliderFlags();

    if (mEntityType == PROJECTILE) {
        // projectiles keep their set velocity
    } else {
        mVelocity.x = mMovement.x * mSpeed;
        mVelocity.y = mMovement.y * mSpeed;
        mVelocity.x += mAcceleration.x * deltaTime;
        mVelocity.y += mAcceleration.y * deltaTime;
    }

    mPosition.x += mVelocity.x * deltaTime;
    if (mEntityType != PROJECTILE) { checkCollisionX(collidableEntities, collisionCheckCount); checkCollisionX(map); }
    mPosition.y += mVelocity.y * deltaTime;
    if (mEntityType != PROJECTILE) { checkCollisionY(collidableEntities, collisionCheckCount); checkCollisionY(map); }

    if (mTextureType == ATLAS && GetLength(mMovement) != 0) animate(deltaTime);
}

// ---- Render ----
void Entity::render()
{
    if (mEntityStatus == INACTIVE) return;
    Rectangle textureArea;
    switch (mTextureType) {
        case SINGLE:
            textureArea = { 0, 0, (float)mTexture.width, (float)mTexture.height };
            break;
        case ATLAS:
            textureArea = getUVRectangle(&mTexture,
                mAnimationIndices.empty() ? 0 : mAnimationIndices[mCurrentFrameIndex],
                mSpriteSheetDimensions.x, mSpriteSheetDimensions.y);
            break;
        default: break;
    }
    Rectangle dest = { mPosition.x, mPosition.y, mScale.x, mScale.y };
    Vector2 orig = { mScale.x/2, mScale.y/2 };
    Color tint = (mFlashTimer > 0) ? RED : WHITE;
    DrawTexturePro(mTexture, textureArea, dest, orig, mAngle, tint);
}

void Entity::displayCollider()
{
    Rectangle box = { mPosition.x - mColliderDimensions.x/2, mPosition.y - mColliderDimensions.y/2,
                      mColliderDimensions.x, mColliderDimensions.y };
    DrawRectangleLines(box.x, box.y, box.width, box.height, GREEN);
}
