#pragma once

#include "geometry.hpp"
#include "rng.hpp"
#include "stats.hpp"

namespace game {

enum class BuffType {
    MaxHp,
    Regen,
    Damage,
    MeleeDamage,
    RangedDamage,
    MoveSpeed,
    AttackSpeed,
    Armor
};

enum class BuffTier {
    D = 0,
    C = 1,
    B = 2,
    A = 3
};

struct Rgba {
    unsigned char r = 255;
    unsigned char g = 255;
    unsigned char b = 255;
    unsigned char a = 255;
};

struct BuffOffer {
    BuffType type = BuffType::Damage;
    BuffTier tier = BuffTier::D;
    int price = 0;
    const char* name = "";
    const char* valueText = "";
    Rgba color;
};

struct ShopState {
    BuffOffer offers[4];
    int shopIndex = 0;
    int refreshPrice = 5;
    bool open = false;
    bool showEndless = false;
    char valueStorage[4][32] = {};
};

struct ShopLayout {
    Rect cards[4];
    Rect refresh;
    Rect nextWave;
    Rect endless;
};

enum class ShopClickKind {
    None,
    Bought,
    NotEnoughMoney,
    Refreshed,
    RefreshNotEnoughMoney,
    NextWave,
    Endless
};

struct ShopClickResult {
    ShopClickKind kind = ShopClickKind::None;
    int offerIndex = -1;
};

int buffPrice(BuffTier tier, int shopIndex);
int refreshBasePrice(int shopIndex);

void beginShop(ShopState& shop, PlayerStats& stats, Rng& rng, int finishedWave);
bool tryBuy(ShopState& shop, PlayerStats& stats, int offerIndex);
bool tryRefresh(ShopState& shop, PlayerStats& stats, Rng& rng);
int tickRegen(PlayerStats& stats, float dt);

ShopLayout makeShopLayout(float screenWidth, float screenHeight);
ShopClickResult clickShop(ShopState& shop, PlayerStats& stats, Rng& rng, float x, float y, const ShopLayout& layout);

}
