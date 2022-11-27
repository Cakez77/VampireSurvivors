#include "render_interface.h"

internal void add_transform(Transform t = {})
{
  dunno->transforms.add(t);
}

internal int get_material_idx(Vec4 color)
{
  int idx = 0;
  bool foundMaterial = false;
  
  for(int materialIdx = 0; materialIdx < dunno->materials.count; materialIdx++)
  {
    if(dunno->materials[materialIdx].color == color)
    {
      idx = materialIdx;
      foundMaterial = true;
      break;
    }
  }
  
  if(!foundMaterial)
  {
    Material m = {color};
    
    idx = dunno->materials.add(m);
  }
  
  return idx;
}

void draw_transform(DrawData drawData)
{
  Sprite s = get_sprite(drawData.spriteID);
  Transform t = {};
  t.pos = drawData.pos;
  t.size = drawData.size;
  t.atlasOffset = s.atlasOffset;
  t.spriteSize = s.subSize;
  t.renderOptions = drawData.renderOptions;
  t.materialIdx = get_material_idx(drawData.color);
  add_transform(t);
}

void draw_quad(Vec2 pos, Vec2 size, DrawData drawData)
{
  drawData.pos = pos;
  drawData.size = size;
  
  draw_transform(drawData);
}

void draw_sprite(SpriteID spriteID, Vec2 pos, Vec2 size, DrawData drawData)
{
  drawData.spriteID = spriteID;
  drawData.pos = pos;
  drawData.size = size;
  
  draw_transform(drawData);
}

void draw_line(Vec2 a, Vec2 b, Vec4 color)
{
  // TODO: Optimize this, use angle, rotation in shader
  float lineLength = length(b - a);
  Vec2 direction = normalize(b - a);
  
  draw_quad(a + direction * 0.0f * lineLength, {2.0f, 2.0f}, {.color = color});
  draw_quad(a + direction * 0.1f * lineLength, {2.0f, 2.0f}, {.color = color});
  draw_quad(a + direction * 0.2f * lineLength, {2.0f, 2.0f}, {.color = color});
  draw_quad(a + direction * 0.3f * lineLength, {2.0f, 2.0f}, {.color = color});
  draw_quad(a + direction * 0.4f * lineLength, {2.0f, 2.0f}, {.color = color});
  draw_quad(a + direction * 0.5f * lineLength, {2.0f, 2.0f}, {.color = color});
  draw_quad(a + direction * 0.6f * lineLength, {2.0f, 2.0f}, {.color = color});
  draw_quad(a + direction * 0.7f * lineLength, {2.0f, 2.0f}, {.color = color});
  draw_quad(a + direction * 0.8f * lineLength, {2.0f, 2.0f}, {.color = color});
  draw_quad(a + direction * 0.9f * lineLength, {2.0f, 2.0f}, {.color = color});
  draw_quad(a + direction * 1.0f * lineLength, {2.0f, 2.0f}, {.color = color});
}

void draw_circle(Circle c, Vec4 color)
{
  float angle = 0.32f;
  for(uint32_t i = 0; i < 20; i++)
  {
    Vec2 a = Vec2{c.radius * sinf(i * angle), c.radius * cosf(i * angle)};
    draw_quad(c.pos + a, {2.0f, 2.0f}, {.color = color});
  }
}

void draw_box(Vec2 pos, Vec2 size, Vec4 color, float lineThickness)
{
  // Top Side
  draw_quad(pos - Vec2{0.0f, (size.y - lineThickness) / 2.0f}, 
            {size.x, lineThickness}, {.color = color});
  
  // Left Side
  draw_quad(pos - Vec2{(size.x - lineThickness) / 2.0f}, 
            {lineThickness, size.y}, {.color = color});
  
  // Right Side
  draw_quad(pos + Vec2{(size.x - lineThickness) / 2.0f}, 
            {lineThickness, size.y}, {.color = color});
  
  // Bottom Side
  draw_quad(pos + Vec2{0.0f, (size.y - lineThickness) / 2.0f},
            {size.x, lineThickness}, {.color = color});
}