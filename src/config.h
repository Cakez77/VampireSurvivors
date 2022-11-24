#pragma once
#include "defines.h"

//#############################################################
//                  Memory
//#############################################################
constexpr int TRANSIENT_BUFFER_SIZE = MB(10);

//#############################################################
//                  Rendering
//#############################################################
constexpr int MAX_TRANSFORMS = 1000;
constexpr int MAX_MATERIALS = 100;

//#############################################################
//                  Game
//#############################################################
constexpr IVec2 WORLD_SIZE = {1600, 900};
constexpr float UNIT_SCALE = 1.5f;
constexpr int MAX_ENEMIES = 1000;
constexpr int MAX_ACTIVE_WEAPONS = 128;
constexpr int MAX_DAMAGING_AREAS = 128;
