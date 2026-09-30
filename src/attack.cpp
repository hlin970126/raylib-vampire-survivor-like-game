#include "attack.hpp"

#include <algorithm>
#include <cmath>

namespace game {

namespace {

bool segmentHitsRect(float x0, float y0, float x1, float y1, const Rect& rect, float& enterTime) {
    float dx = x1 - x0;
    float dy = y1 - y0;
    float tMin = 0.f;
    float tMax = 1.f;

    float slabs[2][2] = {
        {rect.x, rect.x + rect.w},
        {rect.y, rect.y + rect.h}
    };
    float origin[2] = {x0, y0};
    float delta[2] = {dx, dy};

    for (int axis = 0; axis < 2; ++axis) {
        if (std::fabs(delta[axis]) < 1e-8f) {
            if (origin[axis] < slabs[axis][0] || origin[axis] > slabs[axis][1]) {
                return false;
            }
            continue;
        }

        float t1 = (slabs[axis][0] - origin[axis]) / delta[axis];
        float t2 = (slabs[axis][1] - origin[axis]) / delta[axis];
        if (t1 > t2) {
            std::swap(t1, t2);
        }
        if (t1 > tMin) {
            tMin = t1;
        }
        if (t2 < tMax) {
            tMax = t2;
        }
        if (tMin > tMax) {
            return false;
        }
    }

    if (tMax < 0.f || tMin > 1.f) {
        return false;
    }

    enterTime = tMin < 0.f ? 0.f : tMin;
    return true;
}

int nearestTarget(const Rect& player, const std::vector<CombatTarget>& targets) {
    int found = -1;
    float bestDistance = kRangedRange;
    for (int i = 0; i < static_cast<int>(targets.size()); ++i) {
        if (!targets[i].alive) {
            continue;
        }
        float distance = centerDistance(player, targets[i].body);
        if (distance <= bestDistance) {
            bestDistance = distance;
            found = i;
        }
    }
    return found;
}

} // namespace

Rect meleeHitbox(const Rect& player) {
    Rect hitbox;
    hitbox.x = player.x - kMeleePadding;
    hitbox.y = player.y - kMeleePadding;
    hitbox.w = player.w + kMeleePadding * 2.f;
    hitbox.h = player.h + kMeleePadding * 2.f;
    return hitbox;
}

void resetCombat(CombatState& combat) {
    combat.bullets.clear();
    combat.timer = 0.f;
}

void stopCombat(CombatState& combat) {
    combat.bullets.clear();
    combat.timer = attackInterval(PlayerStats{});
}

CombatUpdate updateCombat(CombatState& combat, const PlayerStats& stats, const Rect& player,
                          const std::vector<CombatTarget>& targets, float dt, bool attacking) {
    CombatUpdate result;
    if (dt < 0.f) {
        dt = 0.f;
    }

    if (attacking) {
        combat.timer -= dt;
        if (combat.timer <= 0.f) {
            combat.timer += attackInterval(stats);
            if (combat.timer < 0.f) {
                combat.timer = attackInterval(stats);
            }

            Rect hitbox = meleeHitbox(player);
            float melee = meleeDamage(stats);
            for (int i = 0; i < static_cast<int>(targets.size()); ++i) {
                if (!targets[i].alive || !rectsOverlap(hitbox, targets[i].body)) {
                    continue;
                }
                HitEvent hit;
                hit.targetIndex = i;
                hit.damage = melee;
                result.hits.push_back(hit);
            }

            int targetIndex = nearestTarget(player, targets);
            if (targetIndex >= 0) {
                float dx = rectCenterX(targets[targetIndex].body) - rectCenterX(player);
                float dy = rectCenterY(targets[targetIndex].body) - rectCenterY(player);
                float length = std::sqrt(dx * dx + dy * dy);
                if (length > 0.001f) {
                    Bullet bullet;
                    bullet.x = rectCenterX(player);
                    bullet.y = rectCenterY(player);
                    bullet.vx = dx / length * kBulletSpeed;
                    bullet.vy = dy / length * kBulletSpeed;
                    bullet.damage = rangedDamage(stats);
                    bullet.alive = true;
                    combat.bullets.push_back(bullet);
                }
            }
        }
    }

    for (Bullet& bullet : combat.bullets) {
        if (!bullet.alive) {
            continue;
        }

        float remainingRange = kRangedRange - bullet.traveled;
        if (remainingRange <= 0.f) {
            bullet.alive = false;
            continue;
        }

        float moveX = bullet.vx * dt;
        float moveY = bullet.vy * dt;
        float step = std::sqrt(moveX * moveX + moveY * moveY);
        if (step > remainingRange && step > 0.f) {
            float scale = remainingRange / step;
            moveX *= scale;
            moveY *= scale;
            step = remainingRange;
        }

        float startX = bullet.x;
        float startY = bullet.y;
        float endX = startX + moveX;
        float endY = startY + moveY;

        int hitIndex = -1;
        float bestTime = 2.f;
        for (int i = 0; i < static_cast<int>(targets.size()); ++i) {
            if (!targets[i].alive) {
                continue;
            }
            float enterTime = 0.f;
            if (!segmentHitsRect(startX, startY, endX, endY, targets[i].body, enterTime)) {
                continue;
            }
            if (enterTime < bestTime) {
                bestTime = enterTime;
                hitIndex = i;
            }
        }

        if (hitIndex >= 0) {
            bullet.x = startX + (endX - startX) * bestTime;
            bullet.y = startY + (endY - startY) * bestTime;
            bullet.traveled += step * bestTime;
            bullet.alive = false;
            HitEvent hit;
            hit.targetIndex = hitIndex;
            hit.damage = bullet.damage;
            result.hits.push_back(hit);
            continue;
        }

        bullet.x = endX;
        bullet.y = endY;
        bullet.traveled += step;
        if (bullet.traveled >= kRangedRange) {
            bullet.alive = false;
        }
    }

    return result;
}

}
