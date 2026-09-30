#include "wave.hpp"

#include "constants.hpp"

#include <cmath>

namespace game {

namespace {

int atLeastOne(int wave) {
    return wave < 1 ? 1 : wave;
}

int ceilMultiply(int value, double factor) {
    return static_cast<int>(std::ceil(static_cast<double>(value) * factor));
}

} // namespace

float spawnInterval(int wave) {
    double interval = kFirstSpawnInterval;
    int steps = atLeastOne(wave) - 1;
    for (int i = 0; i < steps; ++i) {
        interval *= 0.95;
    }
    return static_cast<float>(interval);
}

int spawnCount(int wave) {
    int count = kFirstSpawnCount;
    int steps = atLeastOne(wave) - 1;
    for (int i = 0; i < steps; ++i) {
        count = ceilMultiply(count, 1.05);
    }
    return count;
}

int monsterMaxHp(int wave) {
    int hp = kMonsterBaseHp;
    int steps = (atLeastOne(wave) - 1) / 2;
    for (int i = 0; i < steps; ++i) {
        hp = ceilMultiply(hp, 1.05);
    }
    return hp;
}

int monsterContactDamage(int wave) {
    int damage = kMonsterBaseDamage;
    int steps = (atLeastOne(wave) - 1) / 2;
    for (int i = 0; i < steps; ++i) {
        damage = ceilMultiply(damage, 1.05);
    }
    return damage;
}

int moneyDrop(int wave) {
    return 1 + (atLeastOne(wave) - 1) / 2;
}

WaveEvent tickWave(WaveClock& clock, float dt, float waveDuration) {
    if (waveDuration <= 0.f) {
        waveDuration = kWaveDuration;
    }
    if (!clock.running) {
        return WaveEvent::None;
    }
    if (dt > 0.f) {
        clock.elapsed += dt;
    }
    if (clock.elapsed < waveDuration) {
        return WaveEvent::None;
    }

    clock.elapsed = waveDuration;
    clock.running = false;
    if (!clock.endless && clock.wave >= kFinalWave) {
        return WaveEvent::Victory;
    }
    return WaveEvent::EnterShop;
}

void startNextWave(WaveClock& clock) {
    clock.wave += 1;
    clock.elapsed = 0.f;
    clock.running = true;
}

void startEndless(WaveClock& clock) {
    clock.endless = true;
    startNextWave(clock);
}

bool endlessChoiceAvailable(const WaveClock& clock) {
    return !clock.running && !clock.endless && clock.wave == kEndlessOfferWave;
}

}
