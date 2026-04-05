#include "LevelB.h"

LevelB::LevelB()                                      : Scene { {0.0f}, nullptr   } {}
LevelB::LevelB(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

LevelB::~LevelB() { shutdown(); }

void LevelB::initialise()
{
    mGameState.nextSceneID = -1;

    // AUDIO 
    mGameState.bgm       = LoadMusicStream("assets/bgm.ogg");
    mGameState.jumpSound = LoadSound("assets/jump.ogg");
    mGameState.hitSound  = LoadSound("assets/hit.wav");

    SetMusicVolume(mGameState.bgm, 0.33f);
    PlayMusicStream(mGameState.bgm);

    // MAP 
    mGameState.map = new Map(
        LEVEL_WIDTH, LEVEL_HEIGHT,
        (unsigned int *) mLevelData,
        "assets/tileset.png",
        TILE_DIMENSION,
        3, 1,
        mOrigin
    );

    // PROTAGONIST 
    std::map<Direction, std::vector<int>> catAnimationAtlas = {
        {DOWN,  {  0,  1,  2,  3               }},
        {LEFT,  { 40, 41, 42, 43, 44, 45, 46, 47 }},
        {UP,    { 16, 17, 18, 19               }},
        {RIGHT, { 32, 33, 34, 35, 36, 37, 38, 39 }},
    };

    float sizeRatio = 32.0f / 32.0f;

    mGameState.player = new Entity(
        {mOrigin.x - 500.0f, mOrigin.y - 230.0f},
        {150.0f * sizeRatio, 150.0f},
        "assets/Cat_Sprite_Sheet.png",
        ATLAS,
        ATLAS_DIMENSIONS,
        catAnimationAtlas,
        PLAYER
    );

    mGameState.player->setJumpingPower(550.0f);
    mGameState.player->setColliderDimensions({
        mGameState.player->getScale().x / 2.0f,
        mGameState.player->getScale().y / 1.5f
    });
    mGameState.player->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

    // ENEMIES 
    mGameState.enemyCount = LEVELB_ENEMY_COUNT;
    mGameState.enemies    = new Entity[LEVELB_ENEMY_COUNT];

    // Enemy 0: Wanderer
    mGameState.enemies[0].setTexture("assets/Cat_Sprite_Sheet.png");
    mGameState.enemies[0].setEntityType(NPC);
    mGameState.enemies[0].setTextureType(ATLAS);
    mGameState.enemies[0].setSpriteSheetDimensions(ATLAS_DIMENSIONS);
    mGameState.enemies[0].setAnimationAtlas(catAnimationAtlas);
    mGameState.enemies[0].setDirection(LEFT);
    mGameState.enemies[0].setFrameSpeed(14);
    mGameState.enemies[0].setAIType(FOLLOWER);
    mGameState.enemies[0].setAIState(IDLE);
    mGameState.enemies[0].setScale({TILE_DIMENSION * 1.4f, TILE_DIMENSION * 1.4f});
    mGameState.enemies[0].setColliderDimensions({TILE_DIMENSION, TILE_DIMENSION});
    mGameState.enemies[0].setPosition({mOrigin.x + 250.0f, mOrigin.y - 150.0f});
    mGameState.enemies[0].setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
    mGameState.enemies[0].setSpeed(80);

    // Enemy 1: Follower 
    mGameState.enemies[1].setTexture("assets/Cat_Sprite_Sheet.png");
    mGameState.enemies[1].setEntityType(NPC);
    mGameState.enemies[1].setTextureType(ATLAS);
    mGameState.enemies[1].setSpriteSheetDimensions(ATLAS_DIMENSIONS);
    mGameState.enemies[1].setAnimationAtlas(catAnimationAtlas);
    mGameState.enemies[1].setDirection(LEFT);
    mGameState.enemies[1].setFrameSpeed(14);
    mGameState.enemies[1].setAIType(FOLLOWER);
    mGameState.enemies[1].setAIState(IDLE);
    mGameState.enemies[1].setScale({TILE_DIMENSION * 1.4f, TILE_DIMENSION * 1.4f});
    mGameState.enemies[1].setColliderDimensions({TILE_DIMENSION, TILE_DIMENSION});
    mGameState.enemies[1].setPosition({mOrigin.x + 700.0f, mOrigin.y - 150.0f});
    mGameState.enemies[1].setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
    mGameState.enemies[1].setSpeed(120);
}

void LevelB::update(float deltaTime)
{
    UpdateMusicStream(mGameState.bgm);

    mGameState.player->update(
        deltaTime,
        nullptr,
        mGameState.map,
        nullptr, 0
    );

    for (int i = 0; i < mGameState.enemyCount; i++)
    {
        mGameState.enemies[i].update(
            deltaTime,
            mGameState.player,
            mGameState.map,
            nullptr, 0
        );
    }

    // ENEMY COLLISION 
    if (mGameState.player->checkEnemyCollision(mGameState.enemies, mGameState.enemyCount))
    {
        (*mGameState.lives)--;
        PlaySound(mGameState.hitSound);

        if (*mGameState.lives <= 0)
            mGameState.nextSceneID = 5;
        else
            mGameState.player->setPosition({mOrigin.x - 500.0f, mOrigin.y - 230.0f});
    }

    // FALL OFF MAP 
    if (mGameState.player->getPosition().y > END_GAME_THRESHOLD)
    {
        (*mGameState.lives)--;

        if (*mGameState.lives <= 0)
            mGameState.nextSceneID = 5;
        else
            mGameState.player->setPosition({mOrigin.x - 500.0f, mOrigin.y - 230.0f});
    }

    // LEVEL CLEAR 
    float rightEdge = mGameState.map->getRightBoundary() - TILE_DIMENSION * 2;
    if (mGameState.player->getPosition().x > rightEdge)
        mGameState.nextSceneID = 3; // LevelC
}

void LevelB::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    mGameState.player->render();
    mGameState.map->render();

    for (int i = 0; i < mGameState.enemyCount; i++)
        mGameState.enemies[i].render();
}

void LevelB::shutdown()
{
    delete   mGameState.player;
    delete[] mGameState.enemies;
    delete   mGameState.map;

    mGameState.player  = nullptr;
    mGameState.enemies = nullptr;
    mGameState.map     = nullptr;

    UnloadMusicStream(mGameState.bgm);
    UnloadSound(mGameState.jumpSound);
    UnloadSound(mGameState.hitSound);
}
