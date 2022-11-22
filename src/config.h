#pragma once
#include "defines.h"

//#############################################################
//                  File I/O
//#############################################################
constexpr uint32_t FILE_IO_BUFFER_SIZE = MB(5);
constexpr uint32_t TRANSIENT_BUFFER_SIZE = MB(5);

//#############################################################
//                  Rendering
//#############################################################
constexpr uint32_t MAX_TRANSFORMS = 1000;
constexpr uint32_t MAX_MATERIALS = 100;

//#############################################################
//                  Game
//#############################################################
constexpr IVec2 WORLD_SIZE = {1600, 900};
constexpr float UNIT_SCALE = 1.5f;
constexpr uint32_t MAX_WAYPOINTS = 10;
constexpr uint32_t MAX_ENEMIES = 1000;
