
// Output
layout (location = 0) out int renderOptions;
layout (location = 1) out vec4 color;
layout (location = 2) out vec2 textureCoordsOut;

// Input Uniforms
uniform vec2 screenSize;

// Input Buffers
layout(std430, binding = 0) buffer TransfromSBO
{
    Transform transforms[];
};

layout(std430, binding = 1) buffer MaterialSBO
{
    Material materials[];
};

void main()
{
  Transform t = transforms[gl_InstanceID];

  // If not drawing font, then we align to center
  if(!bool(t.renderOptions & RENDER_OPTION_FONT))
  {
    t.pos -= t.size / 2.0;
  }
  renderOptions = t.renderOptions;

  // Inverts the Y- Coordinate, so that y = 0 is on the top

  vec2 vertices[6] = 
  {
    t.pos,                             // Top Left
    vec2(t.pos + vec2(0.0, t.size.y)), // Bottom Left
    vec2(t.pos + vec2(t.size.x, 0.0)), // Top Right
    vec2(t.pos + vec2(t.size.x, 0.0)), // Top Right
    vec2(t.pos + vec2(0.0, t.size.y)), // Bottom Left
    t.pos + t.size                     // Bottom Right
  };

  // Normalize Position
  {
    vec2 vertexPos = vertices[gl_VertexID];
    vertexPos.y = -vertexPos.y + screenSize.y;
    vertexPos = 2.0 * (vertexPos / screenSize) - 1.0;
    gl_Position = vec4(vertexPos, 0.0, 1.0);
  }

  color = materials[t.materialIdx].color;

  // Texture Coords, with flipping
  float left;
  float right;
  float top;
  float bottom;

  float coordOffset = 0.6;
  if(bool(t.renderOptions & RENDER_OPTION_FONT))
  {
    coordOffset = 0.5;
  }
  
  // 0.6 because I HATE OPENGL IT'S SHIT!
  if(bool(t.renderOptions & RENDER_OPTION_FLIP_X))
  {
    left = float(t.atlasOffset.x + t.spriteSize.x) - coordOffset;
    right = float(t.atlasOffset.x) + coordOffset;
  }
  else
  {
    left = float(t.atlasOffset.x) + coordOffset;
    right = float(t.atlasOffset.x + t.spriteSize.x) - coordOffset;
  }
  if(bool(t.renderOptions & RENDER_OPTION_FLIP_Y))
  {
    top = float(t.atlasOffset.y + t.spriteSize.y) - coordOffset;
    bottom = float(t.atlasOffset.y) + coordOffset;
  }
  else
  {
    top = float(t.atlasOffset.y) + coordOffset;
    bottom = float(t.atlasOffset.y + t.spriteSize.y) - coordOffset;
  }
  vec2 textureCoords[6] = 
  {
    vec2(left, top),                                 
    vec2(left, bottom),
    vec2(right, top),
    vec2(right, top),
    vec2(left, bottom),
    vec2(right, bottom)
  };
  textureCoordsOut = vec2(textureCoords[gl_VertexID]);
}