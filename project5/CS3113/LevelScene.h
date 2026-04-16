#ifndef LEVEL_SCENE_H
#define LEVEL_SCENE_H

#include "BattleScene.h"

constexpr int MAX_ENEMIES     = 12;
constexpr int MAX_PICKUPS     = 6;

class LevelScene : public Scene
{
public:
    LevelScene(SceneType levelType);
    ~LevelScene();

    void initialise() override;
    void processInput() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;

    // Player state persistence between scenes
    void setPlayerHP(int hp, int maxHP) { mPlayerStartHP = hp; mPlayerStartMaxHP = maxHP; }
    int  getPlayerHP()    const;
    int  getPlayerMaxHP() const;
    void getStack(Ability *out, int *outSize) const;
    void setStack(Ability *in, int size);

    // Battle encounter info
    bool wantsBattle() const { return mWantsBattle; }
    int  getBattleEnemyIndex() const { return mBattleEnemyIndex; }
    const char* getBattleEnemyTexture() const { return mBattleEnemyTexture; }
    const char* getBattleEnemyTextureAt(int i) const { return mBattleEnemyTextures[i]; }
    int  getBattleEnemyCount() const { return mBattleEnemyIndexCount > 0 ? mBattleEnemyIndexCount : 1; }
    std::string getBattleEnemyName() const { return mBattleEnemyName; }
    int  getBattleEnemyHP() const { return mBattleEnemyHP; }
    int  getBattleEnemyDamage() const { return mBattleEnemyDamage; }
    Texture2D getPlayerTexture() const;
    void clearBattle() { mWantsBattle = false; }

    // Remove defeated enemy after battle
    void removeEnemy(int index);

    // HP ratio for shader (read by main.cpp)
    float getPlayerHPRatio() const;

private:
    SceneType mLevelType;

    Entity *mPlayer = nullptr;
    Entity  mEnemies[MAX_ENEMIES];
    Entity  mPickups[MAX_PICKUPS];
    int mEnemyCount = 0;
    int mPickupCount = 0;

    Map *mMap = nullptr;
    Camera2D mCamera;
    unsigned int *mLevelData = nullptr;

    // Call stack (shared with battle)
    Ability mCallStack[MAX_STACK];
    int mStackSize = 0;

    int mPlayerStartHP = 100;
    int mPlayerStartMaxHP = 100;

    // Elevator
    Vector2 mElevatorPosition;

    // Battle trigger
    bool mWantsBattle = false;
    int  mBattleEnemyIndex = -1;
    const char *mBattleEnemyTexture = nullptr;
    std::string mBattleEnemyName;
    int  mBattleEnemyHP = 0;
    int  mBattleEnemyDamage = 0;
    const char *mBattleEnemyTextures[3] = {nullptr, nullptr, nullptr};
    int  mBattleEnemyIndices[3] = {-1, -1, -1};
    int  mBattleEnemyIndexCount = 0;

    void setupLevel1();
    void setupLevel2();
    void setupLevel3();
    void pushAbility(AbilityType type);
    void checkEnemyEncounters();
    void checkPickupCollisions();
    void checkElevatorCollision();
    void renderHUD();
    void renderCallStack();
    int  countActiveEnemies();
};

#endif
