#include "src/modules.hpp"

#include <raylib.h>

// 成品。课堂练习见 main.cpp。

int main() {
    game::App app;

    app.screenWidth = 800;
    app.screenHeight = 600;
    app.playerMaxHp = 20.f;
    app.playerMoveSpeed = 10.f;
    app.waveSeconds = 60.f;
    app.backgroundColor = BLACK;

    InitWindow(app.screenWidth, app.screenHeight, "Survivor");
    SetTargetFPS(60);

    app.playerTexture = LoadTexture("resources/scarfy.png");
    app.enemyTexture = LoadTexture("resources/cat.png");

    game::startGame(app);

    while (!WindowShouldClose()) {
        float dt = game::frameDelta();

        if (app.phase == game::Phase::Combat) {
            game::updateWaveTimer(app, dt);
            game::updatePlayer(app, dt);
            game::updateEnemies(app, dt);
            game::resolveCollision(app, dt);
            game::updateWeapons(app, dt);
            game::resolveDamage(app, dt);
        } else if (app.phase == game::Phase::Shop) {
            game::updateShop(app);
        } else {
            game::updateRestart(app);
        }

        BeginDrawing();
        game::drawBackground(app);
        game::beginWorldDraw(app);
        game::drawRoom(app);
        game::drawPlayer(app);
        game::drawEnemies(app);
        game::drawBullets(app);
        game::endWorldDraw();
        game::drawUI(app);
        EndDrawing();
    }

    game::unloadGame(app);
    CloseWindow();
    return 0;
}
