#include "app.hpp"

#include "damage.hpp"
#include "player_movement.hpp"

#include <algorithm>
#include <cstdio>

namespace game {

namespace {

const char* buffLabel(BuffType type) {
    switch (type) {
    case BuffType::MaxHp: return "Max HP";
    case BuffType::Regen: return "Regen";
    case BuffType::Damage: return "Damage";
    case BuffType::MeleeDamage: return "Melee";
    case BuffType::RangedDamage: return "Ranged";
    case BuffType::MoveSpeed: return "Move";
    case BuffType::AttackSpeed: return "Atk Spd";
    case BuffType::Armor: return "Armor";
    }
    return "";
}

char tierLetter(BuffTier tier) {
    const char letters[] = {'D', 'C', 'B', 'A'};
    return letters[static_cast<int>(tier)];
}

void drawCentered(const char* text, float x, float y, float width, int size, Color color) {
    int textWidth = MeasureText(text, size);
    DrawText(text, static_cast<int>(x + (width - static_cast<float>(textWidth)) * 0.5f), static_cast<int>(y), size, color);
}

void drawActor(Texture2D texture, const Rect& body, Color fallback) {
    if (texture.id == 0) {
        DrawRectangle(static_cast<int>(body.x), static_cast<int>(body.y),
                      static_cast<int>(body.w), static_cast<int>(body.h), fallback);
        return;
    }

    float frameWidth = static_cast<float>(texture.width);
    float frameHeight = static_cast<float>(texture.height);
    if (frameHeight > 0.f && frameWidth > frameHeight * 1.5f) {
        frameWidth = frameHeight;
    }
    Rectangle source = {0.f, 0.f, frameWidth, frameHeight};
    Rectangle dest = {body.x, body.y, body.w, body.h};
    DrawTexturePro(texture, source, dest, Vector2{0.f, 0.f}, 0.f, WHITE);
}

void damageMonster(App& app, int index, float amount) {
    if (index < 0 || index >= static_cast<int>(app.monsters.monsters().size())) {
        return;
    }
    Monster& monster = app.monsters.monsters()[index];
    if (!monster.alive) {
        return;
    }
    if (applyHealthDamage(monster.hp, amount)) {
        monster.alive = false;
        app.stats.money += moneyDrop(app.clock.wave);
    }
}

void removeDeadMonsters(App& app) {
    std::vector<Monster>& monsters = app.monsters.monsters();
    monsters.erase(std::remove_if(monsters.begin(), monsters.end(), [](const Monster& monster) {
        return !monster.alive;
    }), monsters.end());
}

void enterCombat(App& app) {
    app.monsters.startWave(app.clock.wave);
    resetCombat(app.combat);
    app.shop.open = false;
    app.pendingHits.clear();
    app.pendingContacts.clear();
    app.phase = Phase::Combat;
}

void applyParameters(App& app) {
    if (app.playerMaxHp < 1.f) {
        app.playerMaxHp = 1.f;
    }
    if (app.waveSeconds <= 0.f) {
        app.waveSeconds = kWaveDuration;
    }
    app.stats.maxHp = app.playerMaxHp;
    app.stats.hp = app.playerMaxHp;
    app.stats.moveSpeed = app.playerMoveSpeed;
}

} // namespace

float frameDelta() {
    float dt = GetFrameTime();
    if (dt < 0.f) {
        dt = 0.f;
    }
    if (dt > 0.05f) {
        dt = 0.05f;
    }
    return dt;
}

void startGame(App& app) {
    applyParameters(app);
    app.player = playerBodyAtRoomCenter(app.room);
    app.clock = WaveClock{};
    app.shop = ShopState{};
    app.pendingHits.clear();
    app.pendingContacts.clear();
    app.monsters.startWave(1);
    resetCombat(app.combat);
    app.phase = Phase::Combat;
}

void restartGame(App& app) {
    startGame(app);
}

void unloadGame(App& app) {
    if (app.playerTexture.id != 0) {
        UnloadTexture(app.playerTexture);
        app.playerTexture = Texture2D{};
    }
    if (app.enemyTexture.id != 0) {
        UnloadTexture(app.enemyTexture);
        app.enemyTexture = Texture2D{};
    }
}

void updateWaveTimer(App& app, float dt) {
    if (app.phase != Phase::Combat) {
        return;
    }

    WaveEvent event = tickWave(app.clock, dt, app.waveSeconds);
    if (event == WaveEvent::None) {
        return;
    }

    app.monsters.clearField();
    stopCombat(app.combat);
    app.pendingHits.clear();
    app.pendingContacts.clear();
    if (event == WaveEvent::EnterShop) {
        beginShop(app.shop, app.stats, app.rng, app.clock.wave);
        app.phase = Phase::Shop;
        return;
    }
    app.phase = Phase::Victory;
}

void updatePlayer(App& app, float dt) {
    if (app.phase != Phase::Combat) {
        return;
    }
    movePlayer(app.player, app.stats.moveSpeed, moveInputFromWasd(), app.room, dt);
}

void updateEnemies(App& app, float dt) {
    if (app.phase != Phase::Combat) {
        return;
    }
    app.monsters.update(app.clock.elapsed, dt, app.player, app.room, app.rng, app.waveSeconds);
}

void resolveCollision(App& app, float dt) {
    app.pendingContacts.clear();
    if (app.phase != Phase::Combat) {
        return;
    }
    app.pendingContacts = app.monsters.collectContacts(app.player, dt);
}

void updateWeapons(App& app, float dt) {
    app.pendingHits.clear();
    if (app.phase != Phase::Combat) {
        return;
    }

    std::vector<CombatTarget> targets;
    targets.reserve(app.monsters.monsters().size());
    for (const Monster& monster : app.monsters.monsters()) {
        CombatTarget target;
        target.body = monster.body;
        target.alive = monster.alive;
        targets.push_back(target);
    }

    CombatUpdate attacks = updateCombat(app.combat, app.stats, app.player, targets, dt, true);
    app.pendingHits = attacks.hits;
}

void resolveDamage(App& app, float dt) {
    if (app.phase != Phase::Combat) {
        app.pendingHits.clear();
        app.pendingContacts.clear();
        return;
    }

    for (const HitEvent& hit : app.pendingHits) {
        damageMonster(app, hit.targetIndex, hit.damage);
    }
    for (const ContactEvent& contact : app.pendingContacts) {
        int taken = applyArmor(static_cast<float>(contact.rawDamage), app.stats.armor);
        applyHealthDamage(app.stats.hp, static_cast<float>(taken));
    }
    app.pendingHits.clear();
    app.pendingContacts.clear();

    tickRegen(app.stats, dt);
    removeDeadMonsters(app);

    if (app.stats.hp <= 0.f) {
        app.stats.hp = 0.f;
        app.monsters.clearField();
        stopCombat(app.combat);
        app.phase = Phase::Dead;
    }
}

void updateShop(App& app) {
    if (app.phase != Phase::Shop || !IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return;
    }

    Vector2 mouse = GetMousePosition();
    ShopLayout layout = makeShopLayout(static_cast<float>(app.screenWidth), static_cast<float>(app.screenHeight));
    ShopClickResult result = clickShop(app.shop, app.stats, app.rng, mouse.x, mouse.y, layout);
    if (result.kind == ShopClickKind::NextWave) {
        startNextWave(app.clock);
        enterCombat(app);
    } else if (result.kind == ShopClickKind::Endless) {
        startEndless(app.clock);
        enterCombat(app);
    }
}

void updateRestart(App& app) {
    if ((app.phase == Phase::Dead || app.phase == Phase::Victory) && IsKeyPressed(KEY_R)) {
        restartGame(app);
    }
}

void drawBackground(const App& app) {
    ClearBackground(app.backgroundColor);
}

void beginWorldDraw(const App& app) {
    Camera2D camera = {};
    camera.target = {rectCenterX(app.player), rectCenterY(app.player)};
    camera.offset = {app.screenWidth * 0.5f, app.screenHeight * 0.5f};
    camera.zoom = 1.f;
    BeginMode2D(camera);
}

void endWorldDraw() {
    EndMode2D();
}

void drawRoom(const App& app) {
    DrawRectangle(static_cast<int>(app.room.x), static_cast<int>(app.room.y),
                  static_cast<int>(app.room.width), static_cast<int>(app.room.height), Color{32, 32, 36, 255});
    DrawRectangleLines(static_cast<int>(app.room.x), static_cast<int>(app.room.y),
                       static_cast<int>(app.room.width), static_cast<int>(app.room.height), Color{90, 90, 100, 255});
}

void drawPlayer(const App& app) {
    drawActor(app.playerTexture, app.player, Color{70, 150, 255, 255});
}

void drawEnemies(const App& app) {
    for (const Telegraph& telegraph : app.monsters.telegraphs()) {
        DrawRectangleLines(static_cast<int>(telegraph.x), static_cast<int>(telegraph.y),
                           static_cast<int>(kActorSize), static_cast<int>(kActorSize), YELLOW);
    }

    float maxHp = static_cast<float>(monsterMaxHp(app.clock.wave));
    for (const Monster& monster : app.monsters.monsters()) {
        if (!monster.alive) {
            continue;
        }
        drawActor(app.enemyTexture, monster.body, Color{210, 64, 64, 255});
        float ratio = maxHp > 0.f ? monster.hp / maxHp : 0.f;
        if (ratio < 0.f) {
            ratio = 0.f;
        }
        DrawRectangle(static_cast<int>(monster.body.x), static_cast<int>(monster.body.y) - 6, 20, 3, DARKGRAY);
        DrawRectangle(static_cast<int>(monster.body.x), static_cast<int>(monster.body.y) - 6,
                      static_cast<int>(20.f * ratio), 3, GREEN);
    }
}

void drawBullets(const App& app) {
    if (app.phase == Phase::Combat) {
        Rect hitbox = meleeHitbox(app.player);
        DrawRectangleLines(static_cast<int>(hitbox.x), static_cast<int>(hitbox.y),
                           static_cast<int>(hitbox.w), static_cast<int>(hitbox.h), Color{80, 160, 255, 180});
    }
    for (const Bullet& bullet : app.combat.bullets) {
        if (!bullet.alive) {
            continue;
        }
        DrawCircle(static_cast<int>(bullet.x), static_cast<int>(bullet.y), 3.f, YELLOW);
    }
}

void drawUI(const App& app) {
    float timeLeft = app.waveSeconds - app.clock.elapsed;
    if (timeLeft < 0.f) {
        timeLeft = 0.f;
    }
    DrawText(TextFormat("Wave %d   %.1fs", app.clock.wave, timeLeft), 16, 16, 20, WHITE);
    DrawText(TextFormat("HP %.0f/%.0f   Money %d   Armor %d",
                        app.stats.hp, app.stats.maxHp, app.stats.money, app.stats.armor),
             16, 42, 20, WHITE);
    DrawText(TextFormat("Melee %.1f   Ranged %.1f   Speed %.0f   Interval %.2fs",
                        meleeDamage(app.stats), rangedDamage(app.stats),
                        app.stats.moveSpeed, attackInterval(app.stats)),
             16, 68, 20, WHITE);

    if (app.phase == Phase::Shop) {
        ShopLayout layout = makeShopLayout(static_cast<float>(app.screenWidth), static_cast<float>(app.screenHeight));
        for (int i = 0; i < 4; ++i) {
            const BuffOffer& offer = app.shop.offers[i];
            const Rect& card = layout.cards[i];
            Color tier = Color{offer.color.r, offer.color.g, offer.color.b, offer.color.a};
            DrawRectangle(static_cast<int>(card.x), static_cast<int>(card.y),
                          static_cast<int>(card.w), static_cast<int>(card.h), BLACK);
            DrawRectangleLinesEx(Rectangle{card.x, card.y, card.w, card.h}, 3.f, tier);
            drawCentered(buffLabel(offer.type), card.x, card.y + 36.f, card.w, 24, tier);
            char tierText[2] = {tierLetter(offer.tier), '\0'};
            drawCentered(tierText, card.x, card.y + 78.f, card.w, 28, tier);
            char priceText[32];
            std::snprintf(priceText, sizeof(priceText), "%d", offer.price);
            drawCentered(priceText, card.x, card.y + 124.f, card.w, 24, WHITE);
        }

        DrawRectangle(static_cast<int>(layout.refresh.x), static_cast<int>(layout.refresh.y),
                      static_cast<int>(layout.refresh.w), static_cast<int>(layout.refresh.h), Color{24, 24, 28, 255});
        DrawRectangleLinesEx(Rectangle{layout.refresh.x, layout.refresh.y, layout.refresh.w, layout.refresh.h}, 2.f, WHITE);
        char refreshText[32];
        std::snprintf(refreshText, sizeof(refreshText), "Refresh %d", app.shop.refreshPrice);
        drawCentered(refreshText, layout.refresh.x, layout.refresh.y + 12.f, layout.refresh.w, 20, WHITE);

        DrawRectangle(static_cast<int>(layout.nextWave.x), static_cast<int>(layout.nextWave.y),
                      static_cast<int>(layout.nextWave.w), static_cast<int>(layout.nextWave.h), Color{24, 24, 28, 255});
        DrawRectangleLinesEx(Rectangle{layout.nextWave.x, layout.nextWave.y, layout.nextWave.w, layout.nextWave.h}, 2.f, WHITE);
        drawCentered("Next", layout.nextWave.x, layout.nextWave.y + 12.f, layout.nextWave.w, 20, WHITE);

        if (app.shop.showEndless) {
            DrawRectangle(static_cast<int>(layout.endless.x), static_cast<int>(layout.endless.y),
                          static_cast<int>(layout.endless.w), static_cast<int>(layout.endless.h), Color{24, 24, 28, 255});
            DrawRectangleLinesEx(Rectangle{layout.endless.x, layout.endless.y, layout.endless.w, layout.endless.h}, 2.f, ORANGE);
            drawCentered("Endless", layout.endless.x, layout.endless.y + 12.f, layout.endless.w, 20, ORANGE);
        }
    } else if (app.phase == Phase::Dead || app.phase == Phase::Victory) {
        DrawRectangle(0, 0, app.screenWidth, app.screenHeight, Color{0, 0, 0, 160});
        const char* title = app.phase == Phase::Dead ? "YOU DIED" : "CLEARED";
        drawCentered(title, 0.f, 250.f, static_cast<float>(app.screenWidth), 40, WHITE);
        drawCentered("R to restart", 0.f, 310.f, static_cast<float>(app.screenWidth), 24, GRAY);
    }
}

}
