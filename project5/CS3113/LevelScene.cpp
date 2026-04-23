#include "LevelScene.h"

//  Tile constants — tileset is 256x128 = 8 cols x 4 rows of 32px
//  0 = walkable empty, 1+ = tile index (all non-zero are solid)
constexpr unsigned int W  = 25; // grey wall (tile index 25)
constexpr unsigned int P  = 1;  // partition
constexpr unsigned int D  = 4;  // desk with PC (obstacle)
constexpr unsigned int C  = 5;  // cabinet
constexpr unsigned int PL = 6;  // plant
constexpr unsigned int PR = 7;  // printer

constexpr int   LVL_W = 24;
constexpr int   LVL_H = 18;
constexpr float TILE  = 96.0f;

static Vector2 snapToWalkableTile(Vector2 worldPos, const unsigned int *levelData, Vector2 origin)
{
    float left = origin.x - (LVL_W * TILE) / 2.0f;
    float top  = origin.y - (LVL_H * TILE) / 2.0f;

    int col = (int)floor((worldPos.x - left) / TILE);
    int row = (int)floor((worldPos.y - top) / TILE);

    if (col < 0) col = 0; else if (col >= LVL_W) col = LVL_W - 1;
    if (row < 0) row = 0; else if (row >= LVL_H) row = LVL_H - 1;

    // Keep valid positions unchanged.
    if (levelData[row * LVL_W + col] == 0)
        return worldPos;

    // Find a nearby walkable tile center.
    for (int radius = 1; radius < 8; radius++)
    {
        for (int dy = -radius; dy <= radius; dy++)
        {
            for (int dx = -radius; dx <= radius; dx++)
            {
                int c = col + dx, r = row + dy;
                if (c < 0 || c >= LVL_W || r < 0 || r >= LVL_H) continue;
                if (levelData[r * LVL_W + c] != 0) continue;

                return {
                    left + c * TILE + TILE * 0.5f,
                    top + r * TILE + TILE * 0.5f
                };
            }
        }
    }

    return worldPos;
}

// Ninja Adventure style anim: row0=down, row1=left, row2=right, row3=up
static std::map<Direction, std::vector<int>> makeAnim4x4()
{
    return {
        {DOWN,  {0,1,2,3}}, {LEFT, {4,5,6,7}}, {RIGHT, {8,9,10,11}}, {UP, {12,13,14,15}},
    };
}

//  Level 1 — Cubicle Farm (24x18)
static unsigned int sLevel1[LVL_H * LVL_W] = {
    W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, D, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, D, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, P, P, P, 0, 0, 0, 0, 0, 0, 0, 0, P, P, P, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, P, 0, 0, 0, 0, 0,PL, 0, 0, 0, 0, 0, 0, P, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, D, 0, 0, 0, 0, 0, 0, C, 0, 0, 0, C, 0, 0, 0, 0, 0, 0, D, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, P, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, P, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, P, P, P, 0, 0,PL, 0, 0, 0,PL, 0, P, P, P, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, D, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, D, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W,
};

//  Level 2 — Server Room (tighter corridors)
static unsigned int sLevel2[LVL_H * LVL_W] = {
    W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W,
    W, 0, 0, 0, W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W, 0, 0, 0, W,
    W, 0, 0, 0, W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, C, C, 0, 0, 0, 0, 0, 0, C, C, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, W, W, 0, 0, 0, 0, 0, 0, 0, W, W, W, W, 0, 0, 0, 0, 0, 0, 0, W, W, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, C, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, C, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, C, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, C, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, W, W, 0, 0, 0, 0, 0, 0, 0, W, W, W, W, 0, 0, 0, 0, 0, 0, 0, W, W, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, W, 0, 0, C, C, 0, 0, 0, 0, 0, 0, C, C, 0, 0, W, 0, 0, 0, W,
    W, 0, 0, 0, W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W,
};

//  Level 3 — Executive Suite (boss arena)
static unsigned int sLevel3[LVL_H * LVL_W] = {
    W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0,PL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,PL, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, W, W, W, W, W, W, W, W, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, W, 0, 0, 0, 0, 0, 0, W, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, W, 0, 0, 0, 0, 0, 0, W, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, W, 0, 0, 0, 0, 0, 0, W, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, W, 0, 0, 0, 0, 0, 0, W, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, W, W, W, 0, 0, W, W, W, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0,PL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,PL, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,
    W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W,
};

//  Helper: init enemy
static void initEnemy(Entity &e, Vector2 pos, const char *tex, const std::string &name,
    AIType ai, AIState state, int speed, int hp, float dmg, float tile)
{
    float sz = tile * 0.85f;
    e = Entity(pos, {sz, sz}, tex, NPC);
    e.setAIType(ai); e.setAIState(state); e.setSpeed(speed);
    e.setHP(hp); e.setDamage(dmg); e.setName(name);
    e.setColliderDimensions({sz*0.5f, sz*0.5f});
    e.setAcceleration({0,0});
}

static const char* getTextureFromEnemyName(const std::string &name)
{
    if (name == "Intern")          return "assets/intern.png";
    if (name == "Manager")         return "assets/manager.png";
    if (name == "Consultant")      return "assets/consultant.png";
    if (name == "CEO")             return "assets/boss.png";
    return "assets/intern.png";
}

LevelScene::LevelScene(SceneType t) : mLevelType(t) {}
LevelScene::~LevelScene() { shutdown(); }

void LevelScene::initialise()
{
    mShouldTransition = false;
    mWantsBattle = false;
    mEnemyCount = 0; mPickupCount = 0;

    unsigned int *src = nullptr;
    switch (mLevelType) {
        case LEVEL_1: src = sLevel1; break;
        case LEVEL_2: src = sLevel2; break;
        case LEVEL_3: src = sLevel3; break;
        default: src = sLevel1; break;
    }

    mLevelData = new unsigned int[LVL_W * LVL_H];
    memcpy(mLevelData, src, sizeof(unsigned int) * LVL_W * LVL_H);

    Vector2 origin = {(LVL_W*TILE)/2, (LVL_H*TILE)/2};

    mMap = new Map(LVL_W, LVL_H, mLevelData, "assets/tileset.png", TILE, 8, 4, origin);

    auto anim = makeAnim4x4();
    float psz = TILE * 0.9f;
    mPlayer = new Entity(
        {origin.x - 300, origin.y + 250}, {psz, psz},
        "assets/player.png", ATLAS, {4,4}, anim, PLAYER
    );
    mPlayer->setSpeed(200); mPlayer->setHP(mPlayerStartHP);
    mPlayer->setColliderDimensions({psz*0.45f, psz*0.45f});
    mPlayer->setAcceleration({0,0});
    // Hack: restore maxHP separately if needed
    if (mPlayerStartMaxHP > 0 && mPlayerStartMaxHP != mPlayerStartHP) {
        mPlayer->setHP(mPlayerStartMaxHP);
        mPlayer->takeDamage(mPlayerStartMaxHP - mPlayerStartHP);
    }

    if (mStackSize == 0) {
        pushAbility(ABILITY_PINK_SLIP);
        pushAbility(ABILITY_COFFEE_BREAK);
    }

    mCamera = {0};
    mCamera.target = mPlayer->getPosition();
    mCamera.offset = {(float)GetScreenWidth()/2, (float)GetScreenHeight()/2};
    // Exploration camera zoom.
    mCamera.zoom = 2.0f;

    switch (mLevelType) {
        case LEVEL_1: setupLevel1(); break;
        case LEVEL_2: setupLevel2(); break;
        case LEVEL_3: setupLevel3(); break;
        default: setupLevel1(); break;
    }
}

void LevelScene::setupLevel1()
{
    Vector2 o = {(LVL_W*TILE)/2, (LVL_H*TILE)/2};
    mElevatorPosition = {o.x + 8*TILE, o.y - 6*TILE};

    initEnemy(mEnemies[0], {o.x-150, o.y-100}, "assets/intern.png", "Intern", FOLLOWER, IDLE, 90, 4, 8, TILE);
    initEnemy(mEnemies[1], {o.x+200, o.y-200}, "assets/intern.png", "Intern", FOLLOWER, IDLE, 100, 4, 8, TILE);
    initEnemy(mEnemies[2], {o.x+100, o.y+100}, "assets/intern.png", "Intern", WANDERER, IDLE, 60, 3, 6, TILE);
    initEnemy(mEnemies[3], {o.x, o.y-50}, "assets/manager.png", "Manager", PATROLLER, WALKING, 70, 8, 12, TILE);
    mEnemies[3].setWaypoints({{o.x-200,o.y-50},{o.x+200,o.y-50},{o.x+200,o.y+150},{o.x-200,o.y+150}});
    mEnemyCount = 4;

    mPickups[0] = Entity({o.x+350, o.y+250}, {TILE*0.6f,TILE*0.6f}, "assets/pickup.png", PICKUP);
    mPickups[0].setColliderDimensions({TILE*0.4f, TILE*0.4f});
    mPickupCount = 1;
}

void LevelScene::setupLevel2()
{
    Vector2 o = {(LVL_W*TILE)/2, (LVL_H*TILE)/2};
    mElevatorPosition = {o.x + 7*TILE, o.y - 6*TILE};

    initEnemy(mEnemies[0], {o.x-200, o.y}, "assets/intern.png", "Intern", FOLLOWER, IDLE, 110, 5, 10, TILE);
    initEnemy(mEnemies[1], {o.x+200, o.y}, "assets/intern.png", "Intern", FOLLOWER, IDLE, 110, 5, 10, TILE);
    initEnemy(mEnemies[2], {o.x-100, o.y+200}, "assets/manager.png", "Manager", PATROLLER, WALKING, 80, 10, 15, TILE);
    mEnemies[2].setWaypoints({{o.x-250,o.y+200},{o.x+250,o.y+200}});
    initEnemy(mEnemies[3], {o.x+100, o.y-200}, "assets/manager.png", "Manager", PATROLLER, WALKING, 80, 10, 15, TILE);
    mEnemies[3].setWaypoints({{o.x-100,o.y-250},{o.x+100,o.y-250},{o.x+100,o.y},{o.x-100,o.y}});
    initEnemy(mEnemies[4], {o.x, o.y+50}, "assets/consultant.png", "Consultant", DISRUPTOR, IDLE, 130, 7, 12, TILE);
    mEnemyCount = 5;

    for (int i = 0; i < mEnemyCount; i++)
        mEnemies[i].setPosition(snapToWalkableTile(mEnemies[i].getPosition(), mLevelData, o));

    mElevatorPosition = snapToWalkableTile(mElevatorPosition, mLevelData, o);

    mPickups[0] = Entity({o.x-300, o.y-200}, {TILE*0.6f,TILE*0.6f}, "assets/pickup.png", PICKUP);
    mPickups[0].setPosition(snapToWalkableTile(mPickups[0].getPosition(), mLevelData, o));
    mPickups[0].setColliderDimensions({TILE*0.4f, TILE*0.4f});
    mPickupCount = 1;
}

void LevelScene::setupLevel3()
{
    Vector2 o = {(LVL_W*TILE)/2, (LVL_H*TILE)/2};
    // CEO office is inside the walled room
    mElevatorPosition = {o.x, o.y - TILE};

    initEnemy(mEnemies[0], {o.x-350, o.y-340}, "assets/intern.png", "Intern", FOLLOWER, IDLE, 120, 6, 12, TILE);
    initEnemy(mEnemies[1], {o.x+350, o.y-340}, "assets/intern.png", "Intern", FOLLOWER, IDLE, 120, 6, 12, TILE);
    initEnemy(mEnemies[2], {o.x-350, o.y+340}, "assets/intern.png", "Intern", FOLLOWER, IDLE, 120, 6, 12, TILE);
    initEnemy(mEnemies[3], {o.x+350, o.y+340}, "assets/intern.png", "Intern", FOLLOWER, IDLE, 120, 6, 12, TILE);
    initEnemy(mEnemies[4], {o.x-220, o.y-120}, "assets/consultant.png", "Consultant", DISRUPTOR, IDLE, 140, 8, 15, TILE);
    initEnemy(mEnemies[5], {o.x+220, o.y+120}, "assets/consultant.png", "Consultant", DISRUPTOR, IDLE, 140, 8, 15, TILE);
    // Boss inside the room
    initEnemy(mEnemies[6], {o.x, o.y-TILE}, "assets/boss.png", "CEO", FOLLOWER, IDLE, 50, 20, 25, TILE);
    mEnemyCount = 7;

    for (int i = 0; i < mEnemyCount; i++)
        mEnemies[i].setPosition(snapToWalkableTile(mEnemies[i].getPosition(), mLevelData, o));

    mElevatorPosition = snapToWalkableTile(mElevatorPosition, mLevelData, o);

    mPickups[0] = Entity({o.x, o.y+300}, {TILE*0.6f,TILE*0.6f}, "assets/pickup.png", PICKUP);
    mPickups[0].setPosition(snapToWalkableTile(mPickups[0].getPosition(), mLevelData, o));
    mPickups[0].setColliderDimensions({TILE*0.4f, TILE*0.4f});
    mPickupCount = 1;
}

//  Stack operations
void LevelScene::pushAbility(AbilityType type)
{
    if (mStackSize >= MAX_STACK) return;
    for (int i = mStackSize; i > 0; i--) mCallStack[i] = mCallStack[i-1];
    mCallStack[0] = makeAbility(type);
    mStackSize++;
}

void LevelScene::getStack(Ability *out, int *outSize) const
{
    *outSize = mStackSize;
    for (int i = 0; i < mStackSize; i++) out[i] = mCallStack[i];
}

void LevelScene::setStack(Ability *in, int size)
{
    mStackSize = size;
    for (int i = 0; i < size; i++) mCallStack[i] = in[i];
}

int LevelScene::getPlayerHP() const { return mPlayer ? mPlayer->getHP() : mPlayerStartHP; }
int LevelScene::getPlayerMaxHP() const { return mPlayer ? mPlayer->getMaxHP() : mPlayerStartMaxHP; }
Texture2D LevelScene::getPlayerTexture() const { return mPlayer->getTexture(); }

void LevelScene::removeEnemy(int index)
{
    if (index == -1 && mBattleEnemyIndexCount > 0)
    {
        for (int i = 0; i < mBattleEnemyIndexCount; i++)
        {
            int enemyIdx = mBattleEnemyIndices[i];
            if (enemyIdx >= 0 && enemyIdx < mEnemyCount) mEnemies[enemyIdx].deactivate();
        }
        mBattleEnemyIndexCount = 0;
        return;
    }

    if (index >= 0 && index < mEnemyCount) mEnemies[index].deactivate();
}

//  Encounter — touch enemy to battle
void LevelScene::checkEnemyEncounters()
{
    for (int i = 0; i < mEnemyCount; i++)
    {
        if (!mEnemies[i].isActive()) continue;
        float dist = Vector2Distance(mPlayer->getPosition(), mEnemies[i].getPosition());
        float hitDist = (mPlayer->getColliderDimensions().x + mEnemies[i].getColliderDimensions().x) / 2;

        if (dist < hitDist)
        {
            mWantsBattle = true;
            mBattleEnemyIndex = i;
            mBattleEnemyIndexCount = 0;
            mBattleEnemyName = "";
            mBattleEnemyHP = 0;
            mBattleEnemyDamage = 0;
            mBattleEnemyTextures[0] = mBattleEnemyTextures[1] = mBattleEnemyTextures[2] = nullptr;

            mBattleEnemyIndices[mBattleEnemyIndexCount++] = i;
            mBattleEnemyName = mEnemies[i].getName();
            mBattleEnemyHP += mEnemies[i].getHP();
            mBattleEnemyDamage += (int)mEnemies[i].getDamage();
            mBattleEnemyTextures[0] = getTextureFromEnemyName(mEnemies[i].getName());
            int strongestScore = mEnemies[i].getHP() + (int)mEnemies[i].getDamage();
            const char *strongestTexture = mBattleEnemyTextures[0];

            // Pull in nearby enemies to create mixed multi-enemy encounters
            // (disabled on Level 1 for a gentler intro floor).
            if (mLevelType != LEVEL_1)
            {
                for (int j = 0; j < mEnemyCount && mBattleEnemyIndexCount < 3; j++)
                {
                    if (j == i || !mEnemies[j].isActive()) continue;

                    float groupDist = Vector2Distance(mEnemies[i].getPosition(), mEnemies[j].getPosition());
                    if (groupDist <= TILE * 2.25f)
                    {
                        int slot = mBattleEnemyIndexCount;
                        mBattleEnemyIndices[slot] = j;
                        mBattleEnemyTextures[slot] = getTextureFromEnemyName(mEnemies[j].getName());
                        mBattleEnemyIndexCount++;
                        mBattleEnemyName += " + " + mEnemies[j].getName();
                        mBattleEnemyHP += mEnemies[j].getHP();
                        mBattleEnemyDamage += (int)mEnemies[j].getDamage();

                        int score = mEnemies[j].getHP() + (int)mEnemies[j].getDamage();
                        if (score > strongestScore) {
                            strongestScore = score;
                            strongestTexture = mBattleEnemyTextures[slot];
                        }
                    }
                }
            }

            mBattleEnemyTexture = strongestTexture;
            mBattleEnemyTextures[0] = strongestTexture;

            // Push player away so they don't immediately re-trigger
            Vector2 diff = Vector2Subtract(mPlayer->getPosition(), mEnemies[i].getPosition());
            float len = GetLength(diff);
            if (len > 0) { diff.x /= len; diff.y /= len; }
            mPlayer->setPosition({
                mPlayer->getPosition().x + diff.x * 60,
                mPlayer->getPosition().y + diff.y * 60
            });
            return;
        }
    }
}

void LevelScene::checkPickupCollisions()
{
    for (int i = 0; i < mPickupCount; i++) {
        if (!mPickups[i].isActive()) continue;
        float dist = Vector2Distance(mPlayer->getPosition(), mPickups[i].getPosition());
        if (dist < TILE && IsKeyPressed(KEY_E)) {
            AbilityType randomAbility = (AbilityType)GetRandomValue(0, 5);
            pushAbility(randomAbility);
            mPickups[i].deactivate();
        }
    }
}

void LevelScene::checkElevatorCollision()
{
    if (Vector2Distance(mPlayer->getPosition(), mElevatorPosition) < TILE * 0.8f) {
        switch (mLevelType) {
            case LEVEL_1: mNextScene = LEVEL_2; break;
            case LEVEL_2: mNextScene = LEVEL_3; break;
            case LEVEL_3: mNextScene = WIN_SCENE; break;
            default: mNextScene = WIN_SCENE; break;
        }
        mShouldTransition = true;
    }
}

int LevelScene::countActiveEnemies()
{
    int c = 0;
    for (int i = 0; i < mEnemyCount; i++) if (mEnemies[i].isActive()) c++;
    return c;
}

//  Input / Update
void LevelScene::processInput()
{
    mPlayer->resetMovement();
    if (IsKeyDown(KEY_W)) mPlayer->moveUp();
    if (IsKeyDown(KEY_S)) mPlayer->moveDown();
    if (IsKeyDown(KEY_A)) mPlayer->moveLeft();
    if (IsKeyDown(KEY_D)) mPlayer->moveRight();
    if (GetLength(mPlayer->getMovement()) > 1.0f) mPlayer->normaliseMovement();
}

void LevelScene::update(float deltaTime)
{
    mPlayer->update(deltaTime, nullptr, mMap, nullptr, 0);
    for (int i = 0; i < mEnemyCount; i++)
        mEnemies[i].update(deltaTime, mPlayer, mMap, nullptr, 0);

    checkEnemyEncounters();
    if (mWantsBattle) return; // main.cpp will handle transition

    checkPickupCollisions();
    checkElevatorCollision();

    mCamera.target = mPlayer->getPosition();
}

//  Render
void LevelScene::render()
{
    // Apply burnout shader if loaded
    BeginMode2D(mCamera);

    if (mMap) mMap->render();

    // Elevator
    DrawRectangle((int)(mElevatorPosition.x - TILE/2), (int)(mElevatorPosition.y - TILE/2),
                  (int)TILE, (int)TILE, (Color){255,200,0,100});
    const char *elbl = (mLevelType == LEVEL_3) ? "CEO OFFICE" : "ELEVATOR";
    DrawText(elbl, (int)(mElevatorPosition.x - MeasureText(elbl,10)/2),
             (int)(mElevatorPosition.y-5), 10, YELLOW);

    for (int i = 0; i < mPickupCount; i++) mPickups[i].render();
    for (int i = 0; i < mEnemyCount; i++) mEnemies[i].render();
    mPlayer->render();

    EndMode2D();

    renderHUD();
    renderCallStack();
}

void LevelScene::renderHUD()
{
    // HP bar
    DrawRectangle(20, 20, 204, 24, DARKGRAY);
    float r = mPlayer->getHPRatio();
    Color c = GREEN;
    if (r < 0.25f) c = RED; else if (r < 0.5f) c = ORANGE;
    DrawRectangle(22, 22, (int)(200*r), 20, c);
    char buf[32]; snprintf(buf, 32, "HP: %d/%d", mPlayer->getHP(), mPlayer->getMaxHP());
    DrawText(buf, 24, 24, 16, WHITE);

    const char *floor = "???";
    switch (mLevelType) {
        case LEVEL_1: floor = "1F - Cubicle Farm"; break;
        case LEVEL_2: floor = "2F - Server Room"; break;
        case LEVEL_3: floor = "3F - Executive Suite"; break;
        default: break;
    }
    DrawText(floor, 20, 52, 20, WHITE);

    char ebuf[32]; snprintf(ebuf, 32, "Enemies: %d", countActiveEnemies());
    DrawText(ebuf, 20, 78, 16, LIGHTGRAY);

    if (Vector2Distance(mPlayer->getPosition(), mElevatorPosition) < TILE*2) {
        const char *h = (mLevelType==LEVEL_3) ? ">> Confront the CEO >>" : ">> Walk to ELEVATOR >>";
        DrawText(h, GetScreenWidth()/2 - MeasureText(h,16)/2, GetScreenHeight()-60, 16, YELLOW);
    }
}

void LevelScene::renderCallStack()
{
    int px = GetScreenWidth()-170, py = 20, sw = 150, sh = 36;
    DrawRectangle(px-8, py-8, sw+16, mStackSize*sh+36, (Color){30,30,30,200});
    DrawText("CALL STACK", px+15, py, 14, WHITE);
    py += 24;
    for (int i = 0; i < mStackSize; i++) {
        Color bg = (i==0) ? (Color){40,80,160,255} : (Color){55,55,55,200};
        Color tc = (i==0) ? YELLOW : LIGHTGRAY;
        DrawRectangle(px, py+i*sh, sw, sh-2, bg);
        if (i==0) DrawText(">", px+4, py+i*sh+4, 14, YELLOW);
        DrawText(mCallStack[i].name, px+18, py+i*sh+4, 12, tc);
        char ebuf[32];
        if (mCallStack[i].energy < 0) snprintf(ebuf, sizeof(ebuf), "E:∞  C:%d", mCallStack[i].turnCost);
        else snprintf(ebuf, sizeof(ebuf), "E:%d/%d C:%d", mCallStack[i].energy, mCallStack[i].maxEnergy, mCallStack[i].turnCost);
        DrawText(ebuf, px+18, py+i*sh+20, 9, GRAY);
    }
    if (mStackSize==0) DrawText("(empty)", px+18, py+4, 12, DARKGRAY);
}

void LevelScene::shutdown()
{
    if (mPlayer) { delete mPlayer; mPlayer = nullptr; }
    if (mMap) { delete mMap; mMap = nullptr; }
    if (mLevelData) { delete[] mLevelData; mLevelData = nullptr; }
}

float LevelScene::getPlayerHPRatio() const
{
    return mPlayer ? mPlayer->getHPRatio() : 1.0f;
}

bool LevelScene::shouldShowPickupPrompt() const
{
    if (!mPlayer) return false;

    for (int i = 0; i < mPickupCount; i++)
    {
        if (!mPickups[i].isActive()) continue;
        if (Vector2Distance(mPlayer->getPosition(), mPickups[i].getPosition()) < TILE * 2.5f)
            return true;
    }

    return false;
}
