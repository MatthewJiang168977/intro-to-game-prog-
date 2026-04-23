#ifndef BATTLE_SCENE_H
#define BATTLE_SCENE_H

#include "Scene.h"
#include "raylib.h"
#include <string>

// Ability types
enum AbilityType {
    ABILITY_PINK_SLIP,
    ABILITY_REPLY_ALL,
    ABILITY_COFFEE_BREAK,
    ABILITY_REFACTOR,
    ABILITY_CRUNCH_TIME,
    ABILITY_PTO_REQUEST
};

struct Ability {
    AbilityType type;
    const char *name;
    const char *desc;
    int energy;
    int maxEnergy;
    int turnCost;
};

constexpr int MAX_STACK = 6;

// Shared helper — defined in BattleScene.cpp
Ability makeAbility(AbilityType t);

enum BattleTurn   { PLAYER_CHOOSING, PLAYER_ACTING, ENEMY_ACTING, BATTLE_WON, BATTLE_LOST };
enum BattleResult { RESULT_NONE, RESULT_WIN, RESULT_LOSE };

class BattleScene : public Scene
{
public:
    BattleScene();
    ~BattleScene();

    void initialise() override;
    void processInput() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;

    // Audio injection from main (sounds are owned/unloaded by main)
    void setSounds(Sound attackSfx, Sound hitSfx) { mAttackSfx = attackSfx; mHitSfx = hitSfx; }

    // Set up before battle starts
    void setPlayerData(int hp, int maxHP, Ability *stack, int stackSize, 
                       Texture2D playerTex);
    void setEnemyData(const std::string &name, int hp, int damage,
                      const char **texturePaths, int enemyCount, int enemyIndex);

    // Get results after battle
    BattleResult getResult() const { return mResult; }
    int  getPlayerHP()    const { return mPlayerHP; }
    int  getPlayerMaxHP() const { return mPlayerMaxHP; }
    void getStack(Ability *out, int *outSize) const;
    int  getEnemyIndex()  const { return mEnemyIndex; }
    SceneType getReturnScene() const { return mReturnScene; }
    void setReturnScene(SceneType s) { mReturnScene = s; }

private:
    // Player state
    int mPlayerHP, mPlayerMaxHP;
    Ability mStack[MAX_STACK];
    int mStackSize;
    Texture2D mPlayerTex;

    // Enemy state
    std::string mEnemyName;
    int mEnemyHP, mEnemyMaxHP, mEnemyDamage;
    Texture2D mEnemyTex;
    Texture2D mEnemyGroupTex[3] = {{0}, {0}, {0}};
    int mEnemyGroupCount = 1;
    int mEnemyIndex; // index in exploration map enemy array

    // Battle flow
    BattleTurn   mTurn;
    BattleResult mResult;
    SceneType    mReturnScene;

    int   mCursorPos;     // which stack slot is selected
    float mActionTimer;   // pause between turns for visual feedback
    float mShakeTimer;    // screen shake on hit
    float mShakeAmount;
    float mHitFlashTimer = 0.0f;
    int   mTurnEnergy = 0;
    static constexpr int MAX_TURN_ENERGY = 3;

    bool  mCrunchActive;
    float mDamageMultiplier;

    bool  mInvincibleNextTurn; // PTO request

    std::string mBattleLog; // text shown at bottom

    // Sound effects (injected from main, not owned here)
    Sound mAttackSfx = {0};
    Sound mHitSfx    = {0};

    // Helpers
    void executeAbility(int index);
    void enemyAttack();
    void renderHP(int x, int y, int w, int hp, int maxHP, Color col);
    bool hasUsableAbility() const;
};

#endif
