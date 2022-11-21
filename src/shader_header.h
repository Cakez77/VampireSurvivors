#ifdef GAME

#include "my_math.h"
#define vec2 Vec2
#define ivec2 IVec2
#define vec4 Vec4

#endif

struct Transform
{
  int flipX;
  int flipY;
  vec2 pos;
  vec2 size;
  ivec2 atlasOffset;
  ivec2 spriteSize;
};

struct Material
{
  vec4 color;
};