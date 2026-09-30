#include "src/modules.hpp"

#include <raylib.h>

// 课堂练习。循环、窗口开关已经写好。
// 把注释里的语句补进对应位置，保存后重新编译运行。
// 可以一次只补一行，观察画面少了什么。
// 成品在 main_solution.cpp。

int main() {
    game::App app;

    // 参数。窗口大小必须写在 InitWindow 之前。
    // 调试时可以把 waveSeconds 改成 15，方便很快看到购买界面。
    app.screenWidth = 800;
    app.screenHeight = 600;
    app.playerMaxHp = 20.f;
    app.playerMoveSpeed = 10.f;
    app.waveSeconds = 60.f;
    app.backgroundColor = BLACK;

    InitWindow(app.screenWidth, app.screenHeight, "Survivor");
    SetTargetFPS(60);

    // 素材。LoadTexture 把图片读进显存。
    // 路径相对工程目录。读失败时绘制会退回色块。
    // app.playerTexture = LoadTexture("resources/scarfy.png");
    // app.enemyTexture = LoadTexture("resources/cat.png");

    game::startGame(app);

    // 游戏循环。大约每秒 60 次。dt 是距离上一帧过了多少秒。
    while (!WindowShouldClose()) {
        float dt = game::frameDelta();
        (void)dt;

        if (app.phase == game::Phase::Combat) {
            // 这一波要处理的事，建议按这个顺序补上：
            // game::updateWaveTimer(app, dt);     // 波次计时。时间到了进入购买，第 10 波结束则通关
            // game::updatePlayer(app, dt);        // WASD 移动
            // game::updateEnemies(app, dt);       // 敌人生成，并朝主角移动
            // game::resolveCollision(app, dt);    // 碰撞：主角和敌人重叠时记下接触伤害
            // game::updateWeapons(app, dt);       // 近战和远程子弹
            // game::resolveDamage(app, dt);       // 扣血、击杀掉钱、生命回复、死亡
        } else if (app.phase == game::Phase::Shop) {
            // game::updateShop(app);              // 购买、刷新、下一波、无尽
        } else {
            // game::updateRestart(app);           // 死亡或通关后按 R 重开
        }

        BeginDrawing();
        // game::drawBackground(app);              // 背景。房间外面也是这个颜色
        game::beginWorldDraw(app);                 // 镜头跟着主角。下面四行画的是世界里的物体
        // game::drawRoom(app);                    // 房间
        // game::drawPlayer(app);                  // 人物
        // game::drawEnemies(app);                 // 敌人，含生成预警
        // game::drawBullets(app);                 // 子弹，含近战范围
        game::endWorldDraw();                      // 镜头结束。UI 画在屏幕上，不要放进镜头里
        // game::drawUI(app);                      // 血量、金钱、购买界面、结束画面
        EndDrawing();
    }

    game::unloadGame(app);
    CloseWindow();
    return 0;
}
