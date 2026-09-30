#include "damage.hpp"

#include <cmath>

namespace game {

float meleeDamage(const PlayerStats& stats) {
    return kMeleeBaseDamage * (1.f + stats.damageBonus + stats.meleeBonus);
}

float rangedDamage(const PlayerStats& stats) {
    return kRangedBaseDamage * (1.f + stats.damageBonus + stats.rangedBonus);
}

float attackInterval(const PlayerStats& stats) {
    float attackSpeed = 1.f + stats.attackSpeedBonus;
    if (attackSpeed < 0.01f) {
        attackSpeed = 0.01f;
    }
    return 1.f / attackSpeed;
}

int applyArmor(float rawDamage, int armor) {
    if (rawDamage <= 0.f) {
        return 0;
    }

    float reduction = 0.f;
    if (armor > 0) {
        reduction = static_cast<float>(armor) / static_cast<float>(armor + 1);
    }

    float taken = rawDamage * (1.f - reduction);
    return static_cast<int>(std::ceil(static_cast<double>(taken)));
}

bool applyHealthDamage(float& hp, float amount) {
    if (amount <= 0.f) {
        return hp <= 0.f;
    }
    hp -= amount;
    return hp <= 0.f;
}

}
