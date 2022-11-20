#pragma once
#include "my_math.h"

enum TextureID
{
  TEXTURE_ATLAS_01,
  
  TEXTURE_COUNT
};

enum SpriteID
{
  SPRITE_WHITE,
  SPRITE_ENEMY_01,
  SPRITE_ENEMY_02,
  
  SPRITE_COUNT,
};

struct Sprite
{
  IVec2 atlasOffset;
  IVec2 size;
  IVec2 subSize;
  uint32_t framecount = 1;
};

Sprite get_sprite(SpriteID spriteID);


char* get_asset(TextureID textureID, int* width, int* height);
