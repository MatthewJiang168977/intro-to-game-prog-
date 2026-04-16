#ifndef ENTITY_H
#define ENTITY_H

#include "Map.h"

enum Direction    { LEFT, UP, RIGHT, DOWN              };
enum EntityStatus { ACTIVE, INACTIVE                   };
enum EntityType   { PLAYER, BLOCK, PLATFORM, NPC, PROJECTILE, PICKUP, ENTITY_NONE };
enum AIType       { WANDERER, FOLLOWER, PATROLLER, DISRUPTOR };
enum AIState      { WALKING, IDLE, FOLLOWING, DASHING, COOLDOWN };

class Entity
{
private:
    Vector2 mPosition;
    Vector2 mMovement;
    Vector2 mVelocity;
    Vector2 mAcceleration;

    Vector2 mScale;
    Vector2 mColliderDimensions;
    
    Texture2D mTexture;
    TextureType mTextureType;
    Vector2 mSpriteSheetDimensions;
    
    std::map<Direction, std::vector<int>> mAnimationAtlas;
    std::vector<int> mAnimationIndices;
    Direction mDirection;
    int mFrameSpeed;

    int mCurrentFrameIndex = 0;
    float mAnimationTime = 0.0f;

    bool mIsJumping = false;
    float mJumpingPower = 0.0f;

    int mSpeed;
    float mAngle;

    bool mIsCollidingTop    = false;
    bool mIsCollidingBottom = false;
    bool mIsCollidingRight  = false;
    bool mIsCollidingLeft   = false;

    EntityStatus mEntityStatus = ACTIVE;
    EntityType   mEntityType;

    AIType  mAIType;
    AIState mAIState;

    // HP / combat
    int   mHP    = 0;
    int   mMaxHP = 0;
    float mDamage = 1.0f;
    float mFlashTimer = 0.0f;
    float mLifetime = -1.0f;

    // Patrol
    std::vector<Vector2> mWaypoints;
    int mCurrentWaypoint = 0;

    // Cooldown
    float mCooldownTimer = 0.0f;
    float mCooldownDuration = 2.0f;

    // Battle stats
    std::string mName;

    bool isColliding(Entity *other) const;
    void checkCollisionY(Entity *collidableEntities, int collisionCheckCount);
    void checkCollisionY(Map *map);
    void checkCollisionX(Entity *collidableEntities, int collisionCheckCount);
    void checkCollisionX(Map *map);
    
    void resetColliderFlags() 
    {
        mIsCollidingTop    = false;
        mIsCollidingBottom = false;
        mIsCollidingRight  = false;
        mIsCollidingLeft   = false;
    }

    void animate(float deltaTime);
    void AIActivate(Entity *target);
    void AIWander();
    void AIFollow(Entity *target);
    void AIPatrol();
    void AIDisrupt(Entity *target);

public:
    static constexpr int   DEFAULT_SIZE        = 250;
    static constexpr int   DEFAULT_SPEED       = 200;
    static constexpr int   DEFAULT_FRAME_SPEED = 14;
    static constexpr float Y_COLLISION_THRESHOLD = 0.5f;

    Entity();
    Entity(Vector2 position, Vector2 scale, const char *textureFilepath, EntityType entityType);
    Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
        TextureType textureType, Vector2 spriteSheetDimensions, 
        std::map<Direction, std::vector<int>> animationAtlas, EntityType entityType);
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;
    Entity(Entity&& other) noexcept;
    Entity& operator=(Entity&& other) noexcept;
    ~Entity();

    void update(float deltaTime, Entity *player, Map *map, 
        Entity *collidableEntities, int collisionCheckCount);
    void render();
    void normaliseMovement() { Normalise(&mMovement); }

    void jump()       { mIsJumping = true;  }
    void activate()   { mEntityStatus = ACTIVE;   }
    void deactivate() { mEntityStatus = INACTIVE; }
    void displayCollider();

    bool isActive() const { return mEntityStatus == ACTIVE; }

    void moveUp()    { mMovement.y = -1; mDirection = UP;    }
    void moveDown()  { mMovement.y =  1; mDirection = DOWN;  }
    void moveLeft()  { mMovement.x = -1; mDirection = LEFT;  }
    void moveRight() { mMovement.x =  1; mDirection = RIGHT; }
    void resetMovement() { mMovement = { 0.0f, 0.0f }; }

    // HP
    void takeDamage(int amount);
    bool isDead() const { return mHP <= 0 && mMaxHP > 0; }
    void setHP(int hp) { mHP = hp; mMaxHP = hp; }
    void healHP(int amount) { mHP += amount; if (mHP > mMaxHP) mHP = mMaxHP; }
    int  getHP()    const { return mHP; }
    int  getMaxHP() const { return mMaxHP; }
    float getHPRatio() const { return mMaxHP > 0 ? (float)mHP / mMaxHP : 1.0f; }

    void setDamage(float d) { mDamage = d; }
    float getDamage() const { return mDamage; }
    void setLifetime(float t) { mLifetime = t; }
    void setWaypoints(std::vector<Vector2> wp) { mWaypoints = wp; }
    void setCooldownDuration(float d) { mCooldownDuration = d; }
    float getFlashTimer() const { return mFlashTimer; }

    void setName(const std::string &n) { mName = n; }
    std::string getName() const { return mName; }

    // Getters
    Vector2     getPosition()              const { return mPosition;              }
    Vector2     getMovement()              const { return mMovement;              }
    Vector2     getVelocity()              const { return mVelocity;              }
    Vector2     getAcceleration()          const { return mAcceleration;          }
    Vector2     getScale()                 const { return mScale;                 }
    Vector2     getColliderDimensions()    const { return mScale;                 }
    Vector2     getSpriteSheetDimensions() const { return mSpriteSheetDimensions; }
    Texture2D   getTexture()               const { return mTexture;               }
    TextureType getTextureType()           const { return mTextureType;           }
    Direction   getDirection()             const { return mDirection;             }
    int         getFrameSpeed()            const { return mFrameSpeed;            }
    float       getJumpingPower()          const { return mJumpingPower;          }
    bool        isJumping()                const { return mIsJumping;             }
    int         getSpeed()                 const { return mSpeed;                 }
    float       getAngle()                 const { return mAngle;                 }
    EntityType  getEntityType()            const { return mEntityType;            }
    AIType      getAIType()                const { return mAIType;                }
    AIState     getAIState()               const { return mAIState;               }
    bool isCollidingTop()    const { return mIsCollidingTop;    }
    bool isCollidingBottom() const { return mIsCollidingBottom; }
    std::map<Direction, std::vector<int>> getAnimationAtlas() const { return mAnimationAtlas; }

    // Setters
    void setPosition(Vector2 p)           { mPosition = p;                    }
    void setMovement(Vector2 m)           { mMovement = m;                    }
    void setAcceleration(Vector2 a)       { mAcceleration = a;                }
    void setScale(Vector2 s)              { mScale = s;                       }
    void setTexture(const char *fp)       { mTexture = LoadTexture(fp);       }
    void setColliderDimensions(Vector2 d) { mColliderDimensions = d;          }
    void setSpriteSheetDimensions(Vector2 d) { mSpriteSheetDimensions = d;    }
    void setSpeed(int s)                  { mSpeed = s;                       }
    void setFrameSpeed(int s)             { mFrameSpeed = s;                  }
    void setJumpingPower(float p)         { mJumpingPower = p;                }
    void setAngle(float a)                { mAngle = a;                       }
    void setEntityType(EntityType t)      { mEntityType = t;                  }
    void setDirection(Direction d)
    { 
        mDirection = d;
        if (mTextureType == ATLAS && mAnimationAtlas.count(mDirection))
            mAnimationIndices = mAnimationAtlas.at(mDirection);
    }
    void setAIState(AIState s)  { mAIState = s; }
    void setAIType(AIType t)    { mAIType = t;  }
};

#endif
