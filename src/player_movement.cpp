#include "player_movement.hpp"

#include <raylib.h>

#include <cmath>

namespace game {

MoveInput moveInputFromWasd() {
    MoveInput input;
    if (IsKeyDown(KEY_A)) {
        input.x -= 1.f;
    }
    if (IsKeyDown(KEY_D)) {
        input.x += 1.f;
    }
    if (IsKeyDown(KEY_W)) {
        input.y -= 1.f;
    }
    if (IsKeyDown(KEY_S)) {
        input.y += 1.f;
    }
    return input;
}

void movePlayer(Rect& body, float speed, MoveInput input, const Room& room, float dt) {
    float length = std::sqrt(input.x * input.x + input.y * input.y);
    if (length > 0.f && speed > 0.f && dt > 0.f) {
        body.x += (input.x / length) * speed * dt;
        body.y += (input.y / length) * speed * dt;
    }
    clampRectToRoom(body, room);
}

Rect playerBodyAtRoomCenter(const Room& room) {
    Rect body;
    body.w = kActorSize;
    body.h = kActorSize;
    body.x = room.x + (room.width - body.w) * 0.5f;
    body.y = room.y + (room.height - body.h) * 0.5f;
    return body;
}

}
