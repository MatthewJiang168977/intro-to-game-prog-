#include "BattleScene.h"

static const Ability ABILITY_DEFS[] = {
    { ABILITY_PINK_SLIP,    "Pink Slip",    "Fire projectile (1 dmg)", -1, -1, 1 },
    { ABILITY_REPLY_ALL,    "Reply All",    "Heavy hit (3 dmg)",        4,  4, 2 },
    { ABILITY_COFFEE_BREAK, "Coffee Break", "Heal 30 HP",               3,  3, 2 },
    { ABILITY_REFACTOR,     "Refactor",     "Swap top two abilities",  -1, -1, 1 },
    { ABILITY_CRUNCH_TIME,  "Crunch Time",  "2x dmg next, lose 15 HP",  2,  2, 2 },
    { ABILITY_PTO_REQUEST,  "PTO Request",  "Block next enemy attack",  2,  2, 2 },
};

Ability makeAbility(AbilityType t)
{
    for (int i = 0; i < 6; i++)
        if (ABILITY_DEFS[i].type == t) return ABILITY_DEFS[i];
    return ABILITY_DEFS[0];
}

BattleScene::BattleScene() {}
BattleScene::~BattleScene() { shutdown(); }

void BattleScene::initialise()
{
    mTurn = PLAYER_CHOOSING;
    mResult = RESULT_NONE;
    mCursorPos = 0;
    mActionTimer = 0;
    mShakeTimer = 0;
    mShakeAmount = 0;
    mHitFlashTimer = 0;
    mCrunchActive = false;
    mDamageMultiplier = 1.0f;
    mInvincibleNextTurn = false;
    mTurnEnergy = MAX_TURN_ENERGY;
    mBattleLog = "A wild " + mEnemyName + " appeared!";
    mShouldTransition = false;
}

void BattleScene::setPlayerData(int hp, int maxHP, Ability *stack, int stackSize,
                                 Texture2D playerTex)
{
    mPlayerHP = hp;
    mPlayerMaxHP = maxHP;
    mStackSize = stackSize;
    for (int i = 0; i < stackSize; i++) mStack[i] = stack[i];
    mPlayerTex = playerTex;
}

void BattleScene::setEnemyData(const std::string &name, int hp, int damage,
                                const char **texturePaths, int enemyCount, int enemyIndex)
{
    mEnemyName = name;
    mEnemyHP = hp;
    mEnemyMaxHP = hp;
    mEnemyDamage = damage;
    mEnemyGroupCount = enemyCount < 1 ? 1 : (enemyCount > 3 ? 3 : enemyCount);

    for (int i = 0; i < 3; i++) {
        if (mEnemyGroupTex[i].id != 0) {
            UnloadTexture(mEnemyGroupTex[i]);
            mEnemyGroupTex[i] = {0};
        }
    }

    for (int i = 0; i < mEnemyGroupCount; i++) {
        const char *path = texturePaths[i] ? texturePaths[i] : "assets/intern.png";
        mEnemyGroupTex[i] = LoadTexture(path);
    }
    mEnemyTex = mEnemyGroupTex[0];
    mEnemyIndex = enemyIndex;
}

void BattleScene::getStack(Ability *out, int *outSize) const
{
    *outSize = mStackSize;
    for (int i = 0; i < mStackSize; i++) out[i] = mStack[i];
}

// ============================================================
//  Input
// ============================================================
void BattleScene::processInput()
{
    if (mTurn == PLAYER_CHOOSING && mStackSize > 0)
    {
        if (IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_W))
            mCursorPos = (mCursorPos - 1 + mStackSize) % mStackSize;
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
            mCursorPos = (mCursorPos + 1) % mStackSize;
        if (IsKeyPressed(KEY_R)) {
            if (mStackSize >= 2) {
                Ability tmp = mStack[0]; mStack[0] = mStack[1]; mStack[1] = tmp;
                mBattleLog = "Refactored! Swapped top two abilities.";
            }
        }
        if (IsKeyPressed(KEY_ENTER)) {
            executeAbility(mCursorPos);
        }
    }

    if (mTurn == BATTLE_WON || mTurn == BATTLE_LOST)
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            mResult = (mTurn == BATTLE_WON) ? RESULT_WIN : RESULT_LOSE;
            mShouldTransition = true;
            if (mTurn == BATTLE_WON)
                mNextScene = mReturnScene;
            else
                mNextScene = LOSE_SCENE;
        }
    }
}

bool BattleScene::hasUsableAbility() const
{
    for (int i = 0; i < mStackSize; i++) {
        const Ability &a = mStack[i];
        bool hasItemEnergy = (a.energy == -1) || (a.energy > 0);
        if (hasItemEnergy && a.turnCost <= mTurnEnergy) return true;
    }
    return false;
}

// ============================================================
//  Execute ability
// ============================================================
void BattleScene::executeAbility(int index)
{
    if (index >= mStackSize) return;

    Ability &used = mStack[index];
    bool hasItemEnergy = (used.energy == -1) || (used.energy > 0);
    if (!hasItemEnergy || used.turnCost > mTurnEnergy) {
        mBattleLog = "Not enough energy for " + std::string(used.name) + "!";
        if (!hasUsableAbility()) { mTurn = ENEMY_ACTING; mActionTimer = 0.8f; }
        return;
    }

    mTurnEnergy -= used.turnCost;
    if (used.energy > 0) used.energy--;

    switch (used.type)
    {
        case ABILITY_PINK_SLIP:
        {
            int dmg = (int)(1 * mDamageMultiplier);
            mEnemyHP -= dmg;
            mShakeTimer = 0.2f; mShakeAmount = 8;
            mBattleLog = "You fired a Pink Slip! " + std::to_string(dmg) + " damage!";
            break;
        }
        case ABILITY_REPLY_ALL:
        {
            int dmg = (int)(3 * mDamageMultiplier);
            mEnemyHP -= dmg;
            mShakeTimer = 0.3f; mShakeAmount = 12;
            mBattleLog = "Reply All sent! " + std::to_string(dmg) + " damage to everyone!";
            break;
        }
        case ABILITY_COFFEE_BREAK:
        {
            int heal = 30;
            mPlayerHP += heal;
            if (mPlayerHP > mPlayerMaxHP) mPlayerHP = mPlayerMaxHP;
            mBattleLog = "Coffee Break! Healed " + std::to_string(heal) + " HP.";
            break;
        }
        case ABILITY_REFACTOR:
        {
            if (mStackSize >= 2) {
                Ability tmp = mStack[0]; mStack[0] = mStack[1]; mStack[1] = tmp;
            }
            mBattleLog = "Refactored the call stack!";
            mCursorPos = 0;
            break;
        }
        case ABILITY_CRUNCH_TIME:
        {
            mCrunchActive = true;
            mDamageMultiplier = 2.0f;
            mBattleLog = "Crunch Time! Next attack deals 2x damage... but at a cost.";
            break;
        }
        case ABILITY_PTO_REQUEST:
        {
            mInvincibleNextTurn = true;
            mBattleLog = "PTO Request filed! Next enemy attack will be blocked.";
            break;
        }
    }

    if (index == 0)
    {
        Ability first = mStack[0];
        for (int i = 0; i < mStackSize - 1; i++) mStack[i] = mStack[i+1];
        mStack[mStackSize - 1] = first;
    }

    if (used.type != ABILITY_CRUNCH_TIME && mCrunchActive) {
        mCrunchActive = false;
        mDamageMultiplier = 1.0f;
        mPlayerHP -= 15;
        if (mPlayerHP < 0) mPlayerHP = 0;
        mBattleLog += " Crunch penalty: -15 HP!";
    }

    if (mEnemyHP <= 0) {
        mEnemyHP = 0;
        mTurn = BATTLE_WON;
        mBattleLog = mEnemyName + " has been FIRED!";
        return;
    }

    if (mPlayerHP <= 0) {
        mPlayerHP = 0;
        mTurn = BATTLE_LOST;
        mBattleLog = "You've been FIRED!";
        return;
    }

    if (mTurnEnergy <= 0 || !hasUsableAbility()) {
        mTurn = ENEMY_ACTING;
        mActionTimer = 0.8f;
    } else {
        mTurn = PLAYER_CHOOSING;
        mBattleLog += " (" + std::to_string(mTurnEnergy) + " turn energy left)";
    }
}

// ============================================================
//  Enemy attack
// ============================================================
void BattleScene::enemyAttack()
{
    if (mInvincibleNextTurn) {
        mInvincibleNextTurn = false;
        mBattleLog = mEnemyName + " attacked but your PTO blocked it!";
    } else {
        int dmg = mEnemyDamage + GetRandomValue(-2, 2);
        if (dmg < 1) dmg = 1;
        mPlayerHP -= dmg;
        mShakeTimer = 0.2f; mShakeAmount = 6;
        mHitFlashTimer = 0.18f;
        mBattleLog = mEnemyName + " attacks! " + std::to_string(dmg) + " damage!";
    }

    if (mPlayerHP <= 0) {
        mPlayerHP = 0;
        mTurn = BATTLE_LOST;
        mBattleLog = "You've been FIRED!";
    } else {
        mTurn = PLAYER_CHOOSING;
        mCursorPos = 0;
        mTurnEnergy = MAX_TURN_ENERGY;
    }
}

// ============================================================
//  Update
// ============================================================
void BattleScene::update(float deltaTime)
{
    if (mShakeTimer > 0) mShakeTimer -= deltaTime;
    if (mHitFlashTimer > 0) mHitFlashTimer -= deltaTime;

    if (mTurn == ENEMY_ACTING) {
        mActionTimer -= deltaTime;
        if (mActionTimer <= 0) enemyAttack();
    }
}

// ============================================================
//  Render
// ============================================================
void BattleScene::renderHP(int x, int y, int w, int hp, int maxHP, Color col)
{
    DrawRectangle(x, y, w + 4, 18, DARKGRAY);
    float ratio = (maxHP > 0) ? (float)hp / maxHP : 0;
    Color hpCol = col;
    if (ratio < 0.25f) hpCol = RED;
    else if (ratio < 0.5f) hpCol = ORANGE;
    DrawRectangle(x + 2, y + 2, (int)(w * ratio), 14, hpCol);
    char buf[32];
    snprintf(buf, sizeof(buf), "%d/%d", hp, maxHP);
    DrawText(buf, x + 6, y + 2, 12, WHITE);
}

void BattleScene::render()
{
    int sw = GetScreenWidth(), sh = GetScreenHeight();

    float sx = 0, sy = 0;
    if (mShakeTimer > 0) {
        sx = (float)GetRandomValue(-(int)mShakeAmount, (int)mShakeAmount);
        sy = (float)GetRandomValue(-(int)mShakeAmount, (int)mShakeAmount);
    }

    DrawRectangle(0, 0, sw, sh/2 + 20, (Color){28, 32, 58, 255});
    DrawRectangle(0, sh/2 + 20, sw, sh/2, (Color){22, 24, 40, 255});
    for (int x = 0; x < sw; x += 120) {
        DrawRectangle(x + 24, 36, 72, 130, (Color){40, 48, 80, 180});
        DrawRectangle(x + 34, 46, 52, 90, (Color){120, 160, 220, 40});
    }
    DrawRectangle(0, sh/2 + 16, sw, 4, (Color){90, 95, 130, 255});

    float playerScale = 3.0f;
    Rectangle pSrc = { 0, 0, 64, 64 };
    Rectangle pDst = { 120 + sx, (float)(sh/2 - 80) + sy, 64*playerScale, 64*playerScale };
    DrawTexturePro(mPlayerTex, pSrc, pDst, {0,0}, 0, WHITE);

    DrawText("YOU", 80, sh/2 - 120, 18, WHITE);
    renderHP(80, sh/2 - 98, 160, mPlayerHP, mPlayerMaxHP, GREEN);

    float enemyScale = 2.5f;
    float groupWidth = mEnemyGroupCount * (64 * enemyScale + 12);
    float startX = sw - 180 - groupWidth;
    for (int i = 0; i < mEnemyGroupCount; i++) {
        Texture2D tex = mEnemyGroupTex[i];
        float ex = startX + i * (64 * enemyScale + 12) + (mShakeTimer > 0 ? sx : 0);
        float ey = (float)(sh/2 - 84) + (mShakeTimer > 0 ? sy : 0);
        Rectangle eDst = { ex, ey, 64*enemyScale, 64*enemyScale };

        if (tex.id != 0) {
            Rectangle eSrc = { 0, 0, (float)tex.width, (float)tex.height };
            DrawTexturePro(tex, eSrc, eDst, {0,0}, 0, WHITE);
        } else {
            DrawRectangle(ex, ey, eDst.width, eDst.height, (Color){120, 60, 60, 255});
            DrawText("ENEMY", ex + 24, ey + 64, 20, WHITE);
        }
    }

    if (mHitFlashTimer > 0) {
        float alpha = 0.35f * (mHitFlashTimer / 0.18f);
        DrawRectangle(0, 0, sw, sh, Fade(RED, alpha));
    }

    DrawText(mEnemyName.c_str(), sw - 320, sh/2 - 140, 18, WHITE);
    renderHP(sw - 320, sh/2 - 118, 160, mEnemyHP, mEnemyMaxHP, RED);

    int panelY = sh/2 + 30;
    DrawRectangle(20, panelY, sw/2 - 30, 60, (Color){40, 40, 55, 255});
    DrawText(mBattleLog.c_str(), 30, panelY + 8, 16, WHITE);

    int menuX = sw/2 + 10;
    int menuY = panelY;
    int menuW = sw/2 - 30;

    DrawRectangle(menuX, menuY, menuW, sh - menuY - 20, (Color){40, 40, 55, 255});
    DrawText("CALL STACK", menuX + 10, menuY + 6, 16, YELLOW);

    int slotH = 36;
    int startY = menuY + 30;

    for (int i = 0; i < mStackSize; i++)
    {
        bool selected = (i == mCursorPos && mTurn == PLAYER_CHOOSING);
        Color bg = selected ? (Color){60, 80, 160, 255} : (Color){50, 50, 65, 255};
        Color tc = selected ? YELLOW : LIGHTGRAY;

        DrawRectangle(menuX + 4, startY + i * slotH, menuW - 8, slotH - 2, bg);

        if (selected) DrawText(">", menuX + 8, startY + i * slotH + 4, 16, YELLOW);

        DrawText(mStack[i].name, menuX + 28, startY + i * slotH + 4, 14, tc);
        DrawText(mStack[i].desc, menuX + 28, startY + i * slotH + 20, 10, GRAY);

        char tag[32];
        if (mStack[i].energy < 0) snprintf(tag, sizeof(tag), "E:∞ C:%d", mStack[i].turnCost);
        else snprintf(tag, sizeof(tag), "E:%d C:%d", mStack[i].energy, mStack[i].turnCost);
        DrawText(tag, menuX + menuW - 84, startY + i * slotH + 6, 10, (Color){120, 120, 140, 255});
    }

    if (mStackSize == 0)
        DrawText("Stack empty!", menuX + 28, startY + 4, 14, RED);

    if (mTurn == PLAYER_CHOOSING) {
        char tbuf[64];
        snprintf(tbuf, sizeof(tbuf), "Your turn! Energy: %d/%d", mTurnEnergy, MAX_TURN_ENERGY);
        DrawText(tbuf, 30, panelY + 40, 14, YELLOW);
    }
    else if (mTurn == ENEMY_ACTING)
        DrawText("Enemy is acting...", 30, panelY + 40, 14, RED);
    else if (mTurn == BATTLE_WON)
        DrawText("Press ENTER to continue", 30, panelY + 40, 14, GREEN);
    else if (mTurn == BATTLE_LOST)
        DrawText("Press ENTER to continue", 30, panelY + 40, 14, RED);
}

void BattleScene::shutdown()
{
    for (int i = 0; i < 3; i++) {
        if (mEnemyGroupTex[i].id != 0) {
            UnloadTexture(mEnemyGroupTex[i]);
            mEnemyGroupTex[i] = {0};
        }
    }
    mEnemyTex = {0};
}
