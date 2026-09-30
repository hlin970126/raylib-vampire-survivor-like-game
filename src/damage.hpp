#pragma once

#include "stats.hpp"

namespace game {

float meleeDamage(const PlayerStats& stats);
float rangedDamage(const PlayerStats& stats);
float attackInterval(const PlayerStats& stats);

// 减免比例 = 护甲 / (护甲 + 1)。实际伤害 = 向上取整(原始伤害 * (1 - 减免比例))。
int applyArmor(float rawDamage, int armor);

// 扣血。生命降到 0 或以下时返回 true。
bool applyHealthDamage(float& hp, float amount);

}
