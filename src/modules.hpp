#pragma once

// 课堂只需要看 main.cpp。具体规则在 src 里。
// 每一帧按这个顺序调用：
// 计时、移动、敌人、碰撞、武器、伤害；购买阶段改为 updateShop。
// 绘制顺序：背景、房间、人物、敌人、子弹、UI。世界里的物体要放在 beginWorldDraw 和 endWorldDraw 之间。

#include "app.hpp"
