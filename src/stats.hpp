#pragma once

#include "constants.hpp"

#include <vector>

namespace game {

struct RegenEffect {
    float interval = 1.f;
    float timer = 0.f;
};

struct PlayerStats {
    float maxHp = kBaseMaxHp;
    float hp = kBaseMaxHp;
    float moveSpeed = kBaseMoveSpeed;
    int armor = 0;
    int money = 0;
    float damageBonus = 0.f;
    float meleeBonus = 0.f;
    float rangedBonus = 0.f;
    float attackSpeedBonus = 0.f;
    std::vector<RegenEffect> regens;
};

inline void healToFull(PlayerStats& stats) {
    stats.hp = stats.maxHp;
}

}
