#pragma once

#include "geometry.hpp"

namespace game {

struct MoveInput {
    float x = 0.f;
    float y = 0.f;
};

// 读取 WASD。相反方向互相抵消。W 为屏幕上方。
MoveInput moveInputFromWasd();

// 八个方向共用 speed（像素/秒）。斜向会先归一化，因此不会比直线更快。
void movePlayer(Rect& body, float speed, MoveInput input, const Room& room, float dt);

Rect playerBodyAtRoomCenter(const Room& room);

}
