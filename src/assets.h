#pragma once
#include "defines.h"
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
  SPRITE_HERO_KARATE_MAN,
  SPRITE_EFFECT_WHIP,
  
  SPRITE_COUNT,
};

struct Sprite
{
  IVec2 atlasOffset;
  IVec2 size;
  IVec2 subSize;
  uint32_t framecount = 1;
};

internal Sprite get_sprite(SpriteID spriteID)
{
  Sprite s = {};
  switch(spriteID)
  {
    case SPRITE_WHITE:
    {
      s.atlasOffset = {0, 0};
      s.size = {1, 1};
      s.subSize = {1, 1};
      
      break;
    }
    
    case SPRITE_ENEMY_01:
    {
      s.atlasOffset = {0, 16};
      s.size = {32, 32};
      s.subSize = {32, 32};
      
      break;
    }
    
    case SPRITE_ENEMY_02:
    {
      s.atlasOffset = {32, 0};
      s.size = {64, 64};
      s.subSize = {64, 64};
      
      break;
    }
    
    case SPRITE_HERO_KARATE_MAN:
    {
      s.atlasOffset = {96, 0};
      s.size = {48, 48};
      s.subSize = {48, 48};
      
      break;
    }
    
    case SPRITE_EFFECT_WHIP:
    {
      s.atlasOffset = {144, 0};
      s.size = {240, 32};
      s.subSize = {240, 32};
      
      break;
    }
    
    default: 
    {
      //CAKEZ_ASSERT(0, "Unrecognized spriteID: %d", spriteID);
    }
  }
  
  return s;
}


//#############################################################
//                  Textures Interface
//#############################################################
char* get_asset(TextureID textureID, int* width, int* height);
long long get_last_edit_timestamp(TextureID textureID);

