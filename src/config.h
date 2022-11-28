#pragma once
#include "defines.h"

//#############################################################
//                  Memory
//#############################################################
constexpr int TRANSIENT_BUFFER_SIZE = MB(10);
constexpr int PERSISTANT_BUFFER_SIZE = MB(10);

//#############################################################
//                  Rendering
//#############################################################
constexpr int MAX_TRANSFORMS = 1000;
constexpr int MAX_MATERIALS = 100;
constexpr Vec2 ORIGIN_TOP_LEFT = {1.0f, 1.0f};
constexpr Vec2 ORIGIN_CENTER = {0.0f, 0.0f};

//#############################################################
//                  Game
//#############################################################
constexpr IVec2 WORLD_SIZE = {1600, 900};
constexpr float UNIT_SCALE = 1.5f;
constexpr int MAX_ENEMIES = 500;
constexpr int MAX_ACTIVE_ATTACKS = 128;
constexpr int MAX_DAMAGING_AREAS = 128;
constexpr int MAX_PICKUPS = 1000;