#ifdef GAME // Inside Game
#pragma once

#include "my_math.h"
#define vec2 Vec2
#define ivec2 IVec2
#define vec4 Vec4
typedef int RenderOptions;

#else // Inside Shader 

#define BIT(i) 1 << i

#endif

#define RENDER_OPTION_FLIP_X BIT(0)
#define RENDER_OPTION_FLIP_Y BIT(1)


struct Transform
{
  vec2 pos;
  vec2 size;
  ivec2 atlasOffset;
  ivec2 spriteSize;
  int renderOptions;
  int padding;
};

struct Material
{
  vec4 color;
};