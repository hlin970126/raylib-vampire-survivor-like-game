#pragma once

#include "geometry.hpp"
#include "rng.hpp"

#include <vector>

namespace game {

struct Monster {
    Rect body;
    float hp = static_cast<float>(kMonsterBaseHp);
    int contactDamage = kMonsterBaseDamage;
    float contactTimer = 0.f;
    bool alive = true;
};

struct Telegraph {
    float x = 0.f;
    float y = 0.f;
    float spawnAt = 0.f;
};

struct ContactEvent {
    int monsterIndex = -1;
    int rawDamage = 0;
};

class MonsterSystem {
public:
    void startWave(int wave);
    void clearField();

    // 只负责生成和移动。waveElapsed 到达 waveDuration 时清场。
    void update(float waveElapsed, float dt, const Rect& player, const Room& room, Rng& rng, float waveDuration);

    // 主角和怪物矩形有重叠时，按接触间隔记录一次原始伤害。
    std::vector<ContactEvent> collectContacts(const Rect& player, float dt);

    std::vector<Monster>& monsters() { return monsters_; }
    const std::vector<Monster>& monsters() const { return monsters_; }
    const std::vector<Telegraph>& telegraphs() const { return telegraphs_; }

private:
    void planDueCycles(float waveElapsed, Rng& rng, float waveDuration);
    void triggerWarnings(float waveElapsed, const Rect& player, const Room& room, Rng& rng);
    void spawnDue(float waveElapsed);
    bool pickSpawnBody(Rect& out, const Rect& player, const Room& room, Rng& rng) const;

    std::vector<Monster> monsters_;
    std::vector<Telegraph> telegraphs_;
    std::vector<float> pendingWarnings_;
    int wave_ = 1;
    int nextCycle_ = 0;
    bool spawning_ = true;
};

}
