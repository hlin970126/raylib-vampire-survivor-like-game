#pragma once

#include "constants.hpp"

#include <cmath>

namespace game {

struct Rect {
    float x = 0.f;
    float y = 0.f;
    float w = kActorSize;
    float h = kActorSize;
};

struct Room {
    float x = 0.f;
    float y = 0.f;
    float width = kRoomWidth;
    float height = kRoomHeight;
};

inline Room defaultRoom() {
    return Room{};
}

inline float rectCenterX(const Rect& rect) {
    return rect.x + rect.w * 0.5f;
}

inline float rectCenterY(const Rect& rect) {
    return rect.y + rect.h * 0.5f;
}

inline bool rectsOverlap(const Rect& a, const Rect& b) {
    return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
}

inline bool rectContains(const Rect& rect, float px, float py) {
    return px >= rect.x && py >= rect.y && px < rect.x + rect.w && py < rect.y + rect.h;
}

inline float centerDistance(const Rect& a, const Rect& b) {
    float dx = rectCenterX(a) - rectCenterX(b);
    float dy = rectCenterY(a) - rectCenterY(b);
    return std::sqrt(dx * dx + dy * dy);
}

inline void clampRectToRoom(Rect& rect, const Room& room) {
    float minX = room.x;
    float minY = room.y;
    float maxX = room.x + room.width - rect.w;
    float maxY = room.y + room.height - rect.h;
    if (maxX < minX) {
        maxX = minX;
    }
    if (maxY < minY) {
        maxY = minY;
    }
    if (rect.x < minX) {
        rect.x = minX;
    }
    if (rect.y < minY) {
        rect.y = minY;
    }
    if (rect.x > maxX) {
        rect.x = maxX;
    }
    if (rect.y > maxY) {
        rect.y = maxY;
    }
}

}
