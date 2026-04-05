#include "LevelC.h"

LevelC::LevelC()                                      : Scene { {0.0f}, nullptr   } {}
LevelC::LevelC(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

LevelC::~LevelC() { shutdown(); }

void LevelC::initialise()
{
    mGameState.nextSceneID = -1;

    // AUDIO 
    mGameState.bgm       = LoadMusicStream("assets/bgm.ogg");
    mGameState.jumpSound = LoadSound("assets/jump.ogg");
    mGameState.hitSound  = LoadSound("assets/hit.wav");
    mGameState.deathSound = LoadSound("assets/death.ogg");
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
        {mOrigin.x - 500.0f, mOrigin.y - 200.0f},
        {150.0f * sizeRatio, 150.0f},
        "assets/Cat_Sprite_Sheet.png",
        ATLAS,
        ATLAS_DIMENSIONS,
        catAnimationAtlas,
        PLAYER
    );

    mGameState.player->setJumpingPower(800.0f);
    mGameState.player->setColliderDimensions({
        mGameState.player->getScale().x / 2.0f,
        mGameState.player->getScale().y / 1.5f
    });
    mGameState.player->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

    // ENEMIES 
    mGameState.enemyCount = LEVELC_ENEMY_COUNT;
    mGameState.enemies    = new Entity[LEVELC_ENEMY_COUNT];

    // Enemy 0: Wanderer
    mGameState.enemies[0].setTexture("assets/Cat_Sprite_Sheet.png");
    mGameState.enemies[0].setEntityType(NPC);
    mGameState.enemies[0].setTextureType(ATLAS);
    mGameState.enemies[0].setSpriteSheetDimensions(ATLAS_DIMENSIONS);
    mGameState.enemies[0].setAnimationAtlas(catAnimationAtlas);
    mGameState.enemies[0].setDirection(LEFT);
    mGameState.enemies[0].setFrameSpeed(14);
    mGameState.enemies[0].setAIType(WANDERER);
    mGameState.enemies[0].setAIState(IDLE);
    mGameState.enemies[0].setScale({TILE_DIMENSION * 1.4f, TILE_DIMENSION * 1.4f});
    mGameState.enemies[0].setColliderDimensions({TILE_DIMENSION, TILE_DIMENSION});
    mGameState.enemies[0].setPosition({mOrigin.x - 800.0f, mOrigin.y - 150.0f});
    mGameState.enemies[0].setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
    mGameState.enemies[0].setSpeed(70);

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
    mGameState.enemies[1].setPosition({mOrigin.x - 400.0f, mOrigin.y - 150.0f});
    mGameState.enemies[1].setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
    mGameState.enemies[1].setSpeed(140);

    // Enemy 2: Flyer 
    float flyerY = mOrigin.y - 250.0f;

    mGameState.enemies[2].setTexture("assets/Cat_Sprite_Sheet.png");
    mGameState.enemies[2].setEntityType(NPC);
    mGameState.enemies[2].setTextureType(ATLAS);
    mGameState.enemies[2].setSpriteSheetDimensions(ATLAS_DIMENSIONS);
    mGameState.enemies[2].setAnimationAtlas(catAnimationAtlas);
    mGameState.enemies[2].setDirection(LEFT);
    mGameState.enemies[2].setFrameSpeed(14);
    mGameState.enemies[2].setAIType(FLYER);
    mGameState.enemies[2].setAIState(WALKING);
    mGameState.enemies[2].setScale({TILE_DIMENSION * 1.4f, TILE_DIMENSION * 1.4f});
    mGameState.enemies[2].setColliderDimensions({TILE_DIMENSION, TILE_DIMENSION});
    mGameState.enemies[2].setPosition({mOrigin.x + 500.0f, flyerY});
    mGameState.enemies[2].setFlyerOriginY(flyerY);
    mGameState.enemies[2].setFlyerAmplitude(100.0f);
    mGameState.enemies[2].setFlyerFrequency(2.5f);
    mGameState.enemies[2].setSpeed(90);
}

void LevelC::update(float deltaTime)
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

        if (*mGameState.lives <= 0) {
            PlaySound(mGameState.deathSound);
            mGameState.nextSceneID = 5;
        }
        else 
            mGameState.player->setPosition({mOrigin.x - 500.0f, mOrigin.y - 200.0f});
    }

    // FALL OFF MAP 
    if (mGameState.player->getPosition().y > END_GAME_THRESHOLD)
    {
        (*mGameState.lives)--;
        PlaySound(mGameState.deathSound);

        if (*mGameState.lives <= 0)
            mGameState.nextSceneID = 5;
        else
            mGameState.player->setPosition({mOrigin.x - 500.0f, mOrigin.y - 200.0f});
    }

    // LEVEL CLEAR
    float rightEdge = mGameState.map->getRightBoundary() - TILE_DIMENSION * 2;
    if (mGameState.player->getPosition().x > rightEdge)
        mGameState.nextSceneID = 4; // WinScene
}

void LevelC::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    mGameState.player->render();
    mGameState.map->render();

    for (int i = 0; i < mGameState.enemyCount; i++)
        mGameState.enemies[i].render();
}

void LevelC::shutdown()
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
    UnloadSound(mGameState.deathSound);
}
