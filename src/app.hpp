#pragma once

#include "attack.hpp"
#include "buff_shop.hpp"
#include "monster.hpp"
#include "stats.hpp"
#include "wave.hpp"

#include <raylib.h>

#include <vector>

namespace game {

enum class Phase {
    Combat,
    Shop,
    Dead,
    Victory
};

struct App {
    int screenWidth = 800;
    int screenHeight = 600;
    float playerMaxHp = kBaseMaxHp;
    float playerMoveSpeed = kBaseMoveSpeed;
    float waveSeconds = kWaveDuration;
    Color backgroundColor = BLACK;

    Texture2D playerTexture = {};
    Texture2D enemyTexture = {};

    Phase phase = Phase::Combat;
    Room room = defaultRoom();
    Rect player;
    PlayerStats stats;
    WaveClock clock;
    MonsterSystem monsters;
    CombatState combat;
    ShopState shop;
    Rng rng;

    std::vector<HitEvent> pendingHits;
    std::vector<ContactEvent> pendingContacts;
};

float frameDelta();

void startGame(App& app);
void restartGame(App& app);
void unloadGame(App& app);

void updateWaveTimer(App& app, float dt);
void updatePlayer(App& app, float dt);
void updateEnemies(App& app, float dt);
void resolveCollision(App& app, float dt);
void updateWeapons(App& app, float dt);
void resolveDamage(App& app, float dt);
void updateShop(App& app);
void updateRestart(App& app);

void drawBackground(const App& app);
void beginWorldDraw(const App& app);
void endWorldDraw();
void drawRoom(const App& app);
void drawPlayer(const App& app);
void drawEnemies(const App& app);
void drawBullets(const App& app);
void drawUI(const App& app);

}
