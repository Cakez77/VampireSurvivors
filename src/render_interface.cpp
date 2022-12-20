#include "render_interface.h"

// This assumes a global variable "RenderData* renderData" to exist

internal void add_transform(Transform t = {})
{
  renderData->transforms.add(t);
}

internal void add_transparent_transform(Transform t = {})
{
  renderData->transpTransforms.add(t);
}

internal int get_material_idx(Vec4 color)
{
  int idx = 0;
  bool foundMaterial = false;
  
  for(int materialIdx = 0; materialIdx < renderData->materials.count; materialIdx++)
  {
    if(renderData->materials[materialIdx].color == color)
    {
      idx = materialIdx;
      foundMaterial = true;
      break;
    }
  }
  
  if(!foundMaterial)
  {
    Material m = {color};
    
    idx = renderData->materials.add(m);
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

void draw_sliced_sprite(SpriteID spriteID, Vec2 middle, Vec2 size, DrawData drawData)
{
  Sprite s = get_sprite(spriteID);
  float scale = 2.0f;
  float edgeSize = scale * 5.0f;
  float sizeX = size.x < edgeSize? 1.0f: float(int(size.x - edgeSize * 2.0f));
  float sizeY = size.y < edgeSize? 1.0f: float(int(size.y - edgeSize * 2.0f));
  
  Transform t = {};
  t.renderOptions = drawData.renderOptions | RENDER_OPTION_TOP_LEFT;
  t.materialIdx = get_material_idx(drawData.color);
  
  // Draw Top Left 5x5
  Vec2 topLeft = middle - size / 2.0f;
  t.pos = topLeft;
  t.size = vec_2(5.0f * scale);
  t.atlasOffset = s.atlasOffset;
  t.spriteSize = ivec_2(5);
  add_transform(t);
  
  // Draw Top Middle ...x5
  t.pos = topLeft + Vec2{edgeSize};
  t.size = Vec2{sizeX, edgeSize};
  t.atlasOffset = s.atlasOffset + IVec2{5};
  t.spriteSize = {6, 5};
  add_transform(t);
  
  // Draw Top Right  5x5
  t.pos = topLeft + Vec2{edgeSize + sizeX};
  t.size = vec_2(edgeSize);
  t.atlasOffset = s.atlasOffset + IVec2{10};
  t.spriteSize = ivec_2(5);
  add_transform(t);
  
  // Draw Middle Left
  t.pos = topLeft + Vec2{0.0f, edgeSize};
  t.size = {edgeSize, sizeY};
  t.atlasOffset = s.atlasOffset + IVec2{0, 5};
  t.spriteSize = {5, 6};
  add_transform(t);
  
  // Draw Middle Middle
  t.pos = topLeft + vec_2(edgeSize);
  t.size = {sizeX, sizeY};
  t.atlasOffset = s.atlasOffset + IVec2{5, 5};
  t.spriteSize = ivec_2(5);
  add_transform(t);
  
  // Draw Middle Right
  t.pos = topLeft + Vec2{edgeSize + sizeX, edgeSize};
  t.size = {edgeSize, sizeY};
  t.atlasOffset = s.atlasOffset + IVec2{10, 5};
  t.spriteSize = {5, 5};
  add_transform(t);
  
  // Draw Bottom Left
  t.pos = topLeft + Vec2{0.0f, edgeSize + sizeY};
  t.size = vec_2(edgeSize);
  t.atlasOffset = s.atlasOffset + IVec2{0, 10};
  t.spriteSize = ivec_2(5);
  add_transform(t);
  
  // Draw Bottom Middle
  t.pos = topLeft + Vec2{edgeSize, edgeSize + sizeY};
  t.size = Vec2{sizeX, edgeSize};
  t.atlasOffset = s.atlasOffset + IVec2{5, 10};
  t.spriteSize = {5, 5};
  add_transform(t);
  
  // Draw Bottom Right
  t.pos = topLeft + Vec2{edgeSize + sizeX, edgeSize + sizeY};
  t.size = vec_2(edgeSize);
  t.atlasOffset = s.atlasOffset + IVec2{10, 10};
  t.spriteSize = {5, 5};
  add_transform(t);
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

void draw_text(char* text, Vec2 pos, Vec4 color, RenderOptions renderOptions)
{
  if(!text)
  {
    return;
  }
  
  Transform t = {};
  t.materialIdx = get_material_idx(color);
  t.renderOptions = renderOptions | RENDER_OPTION_FONT | RENDER_OPTION_TOP_LEFT;
  
  float xOrigin = pos.x;
  
  while(char c = *(text++))
  {
    Glyph g = renderData->glyphs[c];
    
    if(c == '\n')
    {
      pos.y += g.advance.y;
      pos.x = xOrigin;
    }
    
    // Inside Loop
    t.pos = pos + Vec2{0.0f, (float)g.offset.y};
    t.size = vec_2(g.spriteSize);
    t.atlasOffset = g.textureOffset;
    t.spriteSize = g.spriteSize;
    add_transparent_transform(t);
    
    pos.x += (float)g.advance.x;
  }
}