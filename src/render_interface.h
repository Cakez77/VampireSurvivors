#pragma once
#include "my_math.h"

struct DrawData
{
  Vec2 pos;
  Vec2 size;
  
  Vec4 color;
};

void draw_quad(DrawData drawData);