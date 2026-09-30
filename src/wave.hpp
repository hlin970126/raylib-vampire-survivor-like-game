#pragma once

namespace game {

struct WaveClock {
    int wave = 1;
    float elapsed = 0.f;
    bool endless = false;
    bool running = true;
};

enum class WaveEvent {
    None,
    EnterShop,
    Victory
};

float spawnInterval(int wave);
int spawnCount(int wave);
int monsterMaxHp(int wave);
int monsterContactDamage(int wave);
int moneyDrop(int wave);

// 计时到达 waveDuration 秒后停止。未开启无尽且当前是第 10 波时返回 Victory，否则返回 EnterShop。
WaveEvent tickWave(WaveClock& clock, float dt, float waveDuration);
void startNextWave(WaveClock& clock);
void startEndless(WaveClock& clock);
bool endlessChoiceAvailable(const WaveClock& clock);

}
