#include "buff_shop.hpp"

#include "wave.hpp"

#include <cmath>
#include <cstdio>

namespace game {

namespace {

constexpr int kTierCount = 4;
constexpr int kBuffTypeCount = 8;

int tierIndex(BuffTier tier) {
    return static_cast<int>(tier);
}

BuffType buffTypeAt(int index) {
    return static_cast<BuffType>(index);
}

BuffTier buffTierAt(int index) {
    return static_cast<BuffTier>(index);
}

const char* buffName(BuffType type) {
    switch (type) {
    case BuffType::MaxHp: return "最大生命";
    case BuffType::Regen: return "生命回复";
    case BuffType::Damage: return "伤害";
    case BuffType::MeleeDamage: return "近战伤害";
    case BuffType::RangedDamage: return "远程伤害";
    case BuffType::MoveSpeed: return "移动速度";
    case BuffType::AttackSpeed: return "攻击速度";
    case BuffType::Armor: return "护甲";
    }
    return "";
}

Rgba tierColor(BuffTier tier) {
    switch (tier) {
    case BuffTier::A: return Rgba{255, 161, 0, 255};
    case BuffTier::B: return Rgba{180, 60, 255, 255};
    case BuffTier::C: return Rgba{0, 228, 48, 255};
    case BuffTier::D: return Rgba{255, 255, 255, 255};
    }
    return Rgba{};
}

int maxHpGain(BuffTier tier) {
    const int values[kTierCount] = {1, 2, 3, 4};
    return values[tierIndex(tier)];
}

int regenInterval(BuffTier tier) {
    const int values[kTierCount] = {7, 5, 3, 1};
    return values[tierIndex(tier)];
}

float damageGain(BuffTier tier) {
    const float values[kTierCount] = {0.02f, 0.03f, 0.04f, 0.05f};
    return values[tierIndex(tier)];
}

float meleeGain(BuffTier tier) {
    const float values[kTierCount] = {0.03f, 0.06f, 0.09f, 0.12f};
    return values[tierIndex(tier)];
}

float rangedGain(BuffTier tier) {
    const float values[kTierCount] = {0.02f, 0.04f, 0.06f, 0.08f};
    return values[tierIndex(tier)];
}

int moveSpeedGain(BuffTier tier) {
    const int values[kTierCount] = {1, 2, 3, 4};
    return values[tierIndex(tier)];
}

float attackSpeedGain(BuffTier tier) {
    const float values[kTierCount] = {0.10f, 0.20f, 0.30f, 0.40f};
    return values[tierIndex(tier)];
}

int armorGain(BuffTier tier) {
    const int values[kTierCount] = {1, 2, 3, 4};
    return values[tierIndex(tier)];
}

int priceBase(BuffTier tier) {
    const int values[kTierCount] = {5, 10, 15, 20};
    return values[tierIndex(tier)];
}

int priceGrowth(BuffTier tier) {
    const int values[kTierCount] = {1, 2, 3, 4};
    return values[tierIndex(tier)];
}

void writeValueText(char* dest, BuffType type, BuffTier tier) {
    switch (type) {
    case BuffType::MaxHp:
        std::snprintf(dest, 32, "+%d", maxHpGain(tier));
        break;
    case BuffType::Regen:
        std::snprintf(dest, 32, "每%d秒回复1", regenInterval(tier));
        break;
    case BuffType::Damage:
        std::snprintf(dest, 32, "+%d%%", static_cast<int>(std::lround(damageGain(tier) * 100.f)));
        break;
    case BuffType::MeleeDamage:
        std::snprintf(dest, 32, "+%d%%", static_cast<int>(std::lround(meleeGain(tier) * 100.f)));
        break;
    case BuffType::RangedDamage:
        std::snprintf(dest, 32, "+%d%%", static_cast<int>(std::lround(rangedGain(tier) * 100.f)));
        break;
    case BuffType::MoveSpeed:
        std::snprintf(dest, 32, "+%d像素/秒", moveSpeedGain(tier));
        break;
    case BuffType::AttackSpeed:
        std::snprintf(dest, 32, "+%d%%", static_cast<int>(std::lround(attackSpeedGain(tier) * 100.f)));
        break;
    case BuffType::Armor:
        std::snprintf(dest, 32, "+%d", armorGain(tier));
        break;
    }
}

void applyOffer(PlayerStats& stats, const BuffOffer& offer) {
    switch (offer.type) {
    case BuffType::MaxHp: {
        float gain = static_cast<float>(maxHpGain(offer.tier));
        stats.maxHp += gain;
        stats.hp += gain;
        break;
    }
    case BuffType::Regen: {
        RegenEffect regen;
        regen.interval = static_cast<float>(regenInterval(offer.tier));
        regen.timer = 0.f;
        stats.regens.push_back(regen);
        break;
    }
    case BuffType::Damage:
        stats.damageBonus += damageGain(offer.tier);
        break;
    case BuffType::MeleeDamage:
        stats.meleeBonus += meleeGain(offer.tier);
        break;
    case BuffType::RangedDamage:
        stats.rangedBonus += rangedGain(offer.tier);
        break;
    case BuffType::MoveSpeed:
        stats.moveSpeed += static_cast<float>(moveSpeedGain(offer.tier));
        break;
    case BuffType::AttackSpeed:
        stats.attackSpeedBonus += attackSpeedGain(offer.tier);
        break;
    case BuffType::Armor:
        stats.armor += armorGain(offer.tier);
        break;
    }
}

void rollOffers(ShopState& shop, Rng& rng) {
    int types[kBuffTypeCount];
    int tiers[kTierCount];
    for (int i = 0; i < kBuffTypeCount; ++i) {
        types[i] = i;
    }
    for (int i = 0; i < kTierCount; ++i) {
        tiers[i] = i;
    }
    for (int i = kBuffTypeCount - 1; i > 0; --i) {
        int swapIndex = rng.nextInt(0, i);
        int saved = types[i];
        types[i] = types[swapIndex];
        types[swapIndex] = saved;
    }
    for (int i = kTierCount - 1; i > 0; --i) {
        int swapIndex = rng.nextInt(0, i);
        int saved = tiers[i];
        tiers[i] = tiers[swapIndex];
        tiers[swapIndex] = saved;
    }

    for (int i = 0; i < 4; ++i) {
        BuffOffer& offer = shop.offers[i];
        offer.type = buffTypeAt(types[i]);
        offer.tier = buffTierAt(tiers[i]);
        offer.price = buffPrice(offer.tier, shop.shopIndex);
        offer.name = buffName(offer.type);
        offer.color = tierColor(offer.tier);
        writeValueText(shop.valueStorage[i], offer.type, offer.tier);
        offer.valueText = shop.valueStorage[i];
    }
}

} // namespace

int buffPrice(BuffTier tier, int shopIndex) {
    if (shopIndex < 0) {
        shopIndex = 0;
    }
    return priceBase(tier) + priceGrowth(tier) * shopIndex;
}

int refreshBasePrice(int shopIndex) {
    if (shopIndex < 0) {
        shopIndex = 0;
    }
    double price = 5.0;
    for (int i = 0; i < shopIndex; ++i) {
        price = std::ceil(price * 1.05);
    }
    return static_cast<int>(price);
}

void beginShop(ShopState& shop, PlayerStats& stats, Rng& rng, int finishedWave) {
    if (finishedWave < 1) {
        finishedWave = 1;
    }
    healToFull(stats);
    shop.shopIndex = finishedWave - 1;
    shop.refreshPrice = refreshBasePrice(shop.shopIndex);
    shop.showEndless = finishedWave == kEndlessOfferWave;
    shop.open = true;
    rollOffers(shop, rng);
}

bool tryBuy(ShopState& shop, PlayerStats& stats, int offerIndex) {
    if (!shop.open || offerIndex < 0 || offerIndex >= 4) {
        return false;
    }
    BuffOffer& offer = shop.offers[offerIndex];
    if (stats.money < offer.price) {
        return false;
    }
    stats.money -= offer.price;
    applyOffer(stats, offer);
    return true;
}

bool tryRefresh(ShopState& shop, PlayerStats& stats, Rng& rng) {
    if (!shop.open || stats.money < shop.refreshPrice) {
        return false;
    }
    stats.money -= shop.refreshPrice;
    shop.refreshPrice += 2;
    rollOffers(shop, rng);
    return true;
}

int tickRegen(PlayerStats& stats, float dt) {
    if (dt <= 0.f) {
        return 0;
    }

    int healed = 0;
    for (RegenEffect& regen : stats.regens) {
        if (regen.interval <= 0.f) {
            continue;
        }
        regen.timer += dt;
        while (regen.timer >= regen.interval) {
            regen.timer -= regen.interval;
            if (stats.hp < stats.maxHp) {
                stats.hp += 1.f;
                if (stats.hp > stats.maxHp) {
                    stats.hp = stats.maxHp;
                }
                healed += 1;
            }
        }
    }
    return healed;
}

ShopLayout makeShopLayout(float screenWidth, float screenHeight) {
    const float cardWidth = 150.f;
    const float cardHeight = 180.f;
    const float gap = 20.f;
    const float buttonWidth = 140.f;
    const float buttonHeight = 44.f;
    const float rowWidth = cardWidth * 4.f + gap * 3.f;

    float left = (screenWidth - rowWidth) * 0.5f;
    float top = (screenHeight - cardHeight) * 0.5f - 40.f;

    ShopLayout layout;
    for (int i = 0; i < 4; ++i) {
        layout.cards[i].x = left + static_cast<float>(i) * (cardWidth + gap);
        layout.cards[i].y = top;
        layout.cards[i].w = cardWidth;
        layout.cards[i].h = cardHeight;
    }

    layout.refresh.w = buttonWidth;
    layout.refresh.h = buttonHeight;
    layout.refresh.x = left + (rowWidth - buttonWidth) * 0.5f;
    layout.refresh.y = top + cardHeight + 24.f;

    layout.nextWave.w = buttonWidth;
    layout.nextWave.h = buttonHeight;
    layout.nextWave.x = left + rowWidth - buttonWidth;
    layout.nextWave.y = layout.refresh.y + buttonHeight + 16.f;

    layout.endless = layout.nextWave;
    layout.endless.x = layout.nextWave.x - gap - buttonWidth;
    return layout;
}

ShopClickResult clickShop(ShopState& shop, PlayerStats& stats, Rng& rng, float x, float y, const ShopLayout& layout) {
    ShopClickResult result;
    if (!shop.open) {
        return result;
    }

    for (int i = 0; i < 4; ++i) {
        if (!rectContains(layout.cards[i], x, y)) {
            continue;
        }
        result.offerIndex = i;
        result.kind = tryBuy(shop, stats, i) ? ShopClickKind::Bought : ShopClickKind::NotEnoughMoney;
        return result;
    }

    if (rectContains(layout.refresh, x, y)) {
        result.kind = tryRefresh(shop, stats, rng) ? ShopClickKind::Refreshed : ShopClickKind::RefreshNotEnoughMoney;
        return result;
    }

    if (shop.showEndless && rectContains(layout.endless, x, y)) {
        result.kind = ShopClickKind::Endless;
        return result;
    }

    if (rectContains(layout.nextWave, x, y)) {
        result.kind = ShopClickKind::NextWave;
        return result;
    }

    return result;
}

}
