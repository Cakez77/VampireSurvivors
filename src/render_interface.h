#pragma once
#include "assets.h"
#include "my_math.h"

struct DrawData
{
  bool flipX;
  bool flipY;
  SpriteID spriteID = SPRITE_WHITE;
  Vec2 pos;
  Vec2 size;
  
  Vec4 color;
};

void draw_quad(DrawData drawData);

void renderer_resize();