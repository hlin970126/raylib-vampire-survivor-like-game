#pragma once

#include "damage.hpp"
#include "geometry.hpp"

#include <vector>

namespace game {

struct CombatTarget {
    Rect body;
    bool alive = true;
};

struct Bullet {
    float x = 0.f;
    float y = 0.f;
    float vx = 0.f;
    float vy = 0.f;
    float damage = 0.f;
    float traveled = 0.f;
    bool alive = false;
};

struct HitEvent {
    int targetIndex = -1;
    float damage = 0.f;
};

struct CombatState {
    std::vector<Bullet> bullets;
    float timer = 0.f;
};

struct CombatUpdate {
    std::vector<HitEvent> hits;
};

Rect meleeHitbox(const Rect& player);

void resetCombat(CombatState& combat);
void stopCombat(CombatState& combat);

// 近战与远程共用攻击间隔。远程瞄准中心距离不超过 100 像素的最近敌人。
// 子弹命中后停止，飞满 100 像素仍未命中也会消失。targetIndex 对应当前 targets 数组。
CombatUpdate updateCombat(CombatState& combat, const PlayerStats& stats, const Rect& player,
                          const std::vector<CombatTarget>& targets, float dt, bool attacking);

}
