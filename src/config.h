#pragma once
#include "assets.h"
#include "defines.h"

//#############################################################
//                  Memory
//#############################################################
constexpr int TRANSIENT_BUFFER_SIZE = MB(50);
constexpr int PERSISTENT_BUFFER_SIZE = MB(100);

//#############################################################
//                  Rendering
//#############################################################
constexpr int MAX_TRANSFORMS = 5000;
constexpr int MAX_MATERIALS = 100;
constexpr Vec2 ORIGIN_TOP_LEFT = {1.0f, 1.0f};
constexpr Vec2 ORIGIN_CENTER = {0.0f, 0.0f};

//#############################################################
//                  Audio
//#############################################################
constexpr int MAX_BYTES_SOUND_BUFFER = MB(50);
constexpr int MAX_PLAYING_SOUNDS = 5;
constexpr int MAX_ALLOCATED_SOUNDS = 10;

//#############################################################
//                  Game
//#############################################################
constexpr IVec2 SCREEN_SIZE = {1600, 900};
constexpr IVec2 WORLD_GRID_SIZE = {20, 13};
constexpr int MAP_CHUNK_COUNT = 15;
constexpr int MAP_CHUNK_TILE_COUNT = 15;
constexpr float UNIT_SCALE = 1.5f;
constexpr int MAX_ENEMIES_CHUNK = 50;
constexpr int MAX_ENEMIES = 500;
constexpr int MAX_ACTIVE_ATTACKS = 128;
constexpr int MAX_DAMAGING_AREAS = 128;
constexpr int MAX_PICKUPS = 1000;
constexpr int MAX_DAMAGE_NUMBERS = 200;
constexpr int CRYSTAL_MERGE_COUNT = 50;
constexpr int MAX_OBSTACLES = 50;

internal SpriteID ChunkTiles [MAP_CHUNK_TILE_COUNT][MAP_CHUNK_TILE_COUNT] = 
{
  {TILE_02, TILE_02, TILE_02, TILE_02, TILE_02, TILE_02, TILE_01, TILE_02, TILE_02, TILE_02, TILE_02, TILE_02, TILE_02, TILE_02, TILE_02},
  {TILE_02, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_02},
  {TILE_02, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_02},
  {TILE_02, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_02},
  {TILE_02, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_02},
  {TILE_02, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_02},
  {TILE_02, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_02},
  {TILE_02, TILE_01, TILE_01, TILE_03, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_02},
  {TILE_02, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_02},
  {TILE_02, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_03, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_02},
  {TILE_02, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_03, TILE_01, TILE_01, TILE_01, TILE_02},
  {TILE_02, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_02},
  {TILE_02, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_02},
  {TILE_02, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_01, TILE_03, TILE_01, TILE_02},
  {TILE_02, TILE_02, TILE_02, TILE_02, TILE_02, TILE_02, TILE_02, TILE_02, TILE_02, TILE_02, TILE_02, TILE_02, TILE_02, TILE_02, TILE_02},
};


// EXP Table, needed by Pickups and EXP Bar
internal int expTable [] = 
{
  0,
  20,
  40,
  60,
  80,
  100,
  120,
  140,
  160,
  180,
  200,
  220,
  240,
  260,
  280,
  300,
  320,
  340,
};