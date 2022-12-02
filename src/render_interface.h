#pragma once
#include "assets.h"
#include "colors.h"
#include "config.h"
#include "my_math.h"
#include "shader_header.h"

//#############################################################
//                  Structures
//#############################################################
struct Glyph
{
  IVec2 textureOffset;
  IVec2 spriteSize;
  IVec2 advance;
  IVec2 offset;
};

struct DrawData
{
  SpriteID spriteID = SPRITE_WHITE;
  Vec2 pos;
  Vec2 size;
  Vec2 vertexOffset = ORIGIN_CENTER;
  
  Vec4 color = COLOR_WHITE;
  RenderOptions renderOptions;
};

struct RenderData
{
  Array<Material, MAX_MATERIALS> materials;
  Array<Transform, MAX_TRANSFORMS> transforms;
  Array<Transform, MAX_TRANSFORMS> transpTransforms;
  
  Glyph glyphs[127];
};

//#############################################################
//                  Draw Functions
//#############################################################
void draw_transform(DrawData drawData);
void draw_quad(Vec2 pos, Vec2 size, DrawData drawData = {});
void draw_sprite(SpriteID spriteID, Vec2 pos, Vec2 size, DrawData drawData = {});

void draw_sliced_sprite(SpriteID spriteID, Vec2 pos, Vec2 size, DrawData drawData = {});
void draw_line(Vec2 a, Vec2 b, Vec4 color = COLOR_WHITE);
void draw_circle(Circle c, Vec4 color = COLOR_WHITE);
void draw_box(Vec2 pos, Vec2 size, Vec4 color = COLOR_WHITE, float lineThickness = 5.0f);
void draw_text(char* text, Vec2 pos, Vec4 color = COLOR_WHITE, RenderOptions renderOptions = 0);