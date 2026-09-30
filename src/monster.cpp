#include "monster.hpp"

#include "wave.hpp"

#include <cmath>

namespace game {

void MonsterSystem::startWave(int wave) {
    clearField();
    wave_ = wave < 1 ? 1 : wave;
    nextCycle_ = 0;
    spawning_ = true;
}

void MonsterSystem::clearField() {
    monsters_.clear();
    telegraphs_.clear();
    pendingWarnings_.clear();
    spawning_ = false;
}

void MonsterSystem::planDueCycles(float waveElapsed, Rng& rng, float waveDuration) {
    float interval = spawnInterval(wave_);
    int count = spawnCount(wave_);
    if (interval <= 0.001f) {
        return;
    }

    int lastCycle = static_cast<int>(waveElapsed / interval);
    while (nextCycle_ <= lastCycle) {
        float cycleStart = static_cast<float>(nextCycle_) * interval;
        if (cycleStart >= waveDuration) {
            break;
        }
        for (int i = 0; i < count; ++i) {
            float warnAt = cycleStart + rng.nextFloat(0.f, interval);
            if (warnAt < waveDuration) {
                pendingWarnings_.push_back(warnAt);
            }
        }
        nextCycle_ += 1;
    }
}

bool MonsterSystem::pickSpawnBody(Rect& out, const Rect& player, const Room& room, Rng& rng) const {
    float maxX = room.width - kActorSize;
    float maxY = room.height - kActorSize;
    if (maxX < 0.f || maxY < 0.f) {
        return false;
    }

    for (int attempt = 0; attempt < 64; ++attempt) {
        Rect body;
        body.x = room.x + rng.nextFloat(0.f, maxX);
        body.y = room.y + rng.nextFloat(0.f, maxY);
        body.w = kActorSize;
        body.h = kActorSize;
        if (centerDistance(body, player) >= kSpawnClearance) {
            out = body;
            return true;
        }
    }
    return false;
}

void MonsterSystem::triggerWarnings(float waveElapsed, const Rect& player, const Room& room, Rng& rng) {
    std::vector<float> stillPending;
    stillPending.reserve(pendingWarnings_.size());
    for (float warnAt : pendingWarnings_) {
        if (warnAt > waveElapsed) {
            stillPending.push_back(warnAt);
            continue;
        }

        Rect body;
        if (!pickSpawnBody(body, player, room, rng)) {
            continue;
        }
        Telegraph telegraph;
        telegraph.x = body.x;
        telegraph.y = body.y;
        telegraph.spawnAt = warnAt + kSpawnWarningDuration;
        telegraphs_.push_back(telegraph);
    }
    pendingWarnings_.swap(stillPending);
}

void MonsterSystem::spawnDue(float waveElapsed) {
    std::vector<Telegraph> stillWaiting;
    stillWaiting.reserve(telegraphs_.size());
    for (const Telegraph& telegraph : telegraphs_) {
        if (telegraph.spawnAt > waveElapsed) {
            stillWaiting.push_back(telegraph);
            continue;
        }

        Monster monster;
        monster.body.x = telegraph.x;
        monster.body.y = telegraph.y;
        monster.body.w = kActorSize;
        monster.body.h = kActorSize;
        monster.hp = static_cast<float>(monsterMaxHp(wave_));
        monster.contactDamage = monsterContactDamage(wave_);
        monster.alive = true;
        monsters_.push_back(monster);
    }
    telegraphs_.swap(stillWaiting);
}

void MonsterSystem::update(float waveElapsed, float dt, const Rect& player, const Room& room, Rng& rng, float waveDuration) {
    if (waveDuration <= 0.f) {
        waveDuration = kWaveDuration;
    }
    if (waveElapsed >= waveDuration) {
        clearField();
        return;
    }
    if (dt < 0.f) {
        dt = 0.f;
    }

    if (spawning_) {
        planDueCycles(waveElapsed, rng, waveDuration);
        triggerWarnings(waveElapsed, player, room, rng);
        spawnDue(waveElapsed);
    }

    for (Monster& monster : monsters_) {
        if (!monster.alive) {
            continue;
        }

        float dx = rectCenterX(player) - rectCenterX(monster.body);
        float dy = rectCenterY(player) - rectCenterY(monster.body);
        float length = std::sqrt(dx * dx + dy * dy);
        if (length > 0.001f) {
            monster.body.x += dx / length * kMonsterSpeed * dt;
            monster.body.y += dy / length * kMonsterSpeed * dt;
        }
        clampRectToRoom(monster.body, room);
    }
}

std::vector<ContactEvent> MonsterSystem::collectContacts(const Rect& player, float dt) {
    std::vector<ContactEvent> contacts;
    if (dt < 0.f) {
        dt = 0.f;
    }

    for (int i = 0; i < static_cast<int>(monsters_.size()); ++i) {
        Monster& monster = monsters_[i];
        if (!monster.alive) {
            continue;
        }
        if (!rectsOverlap(monster.body, player)) {
            monster.contactTimer = 0.f;
            continue;
        }

        monster.contactTimer -= dt;
        if (monster.contactTimer <= 0.f) {
            ContactEvent event;
            event.monsterIndex = i;
            event.rawDamage = monster.contactDamage;
            contacts.push_back(event);
            monster.contactTimer += kContactInterval;
            if (monster.contactTimer < 0.f) {
                monster.contactTimer = kContactInterval;
            }
        }
    }
    return contacts;
}

}
