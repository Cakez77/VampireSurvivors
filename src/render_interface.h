#pragma once
#include "assets.h"
#include "my_math.h"
#include "shader_header.h"

//#############################################################
//                  Structures
//#############################################################
struct DrawData
{
  SpriteID spriteID = SPRITE_WHITE;
  Vec2 pos;
  Vec2 size;
  
  Vec4 color;
  RenderOptions renderOptions;
};

//#############################################################
//                  Draw Functions
//#############################################################
void draw_quad(DrawData drawData);


//#############################################################
//                  Utility Functions
//#############################################################
bool renderer_get_vertical_sync();
void renderer_set_vertical_sync(bool vSync);
void renderer_resize();