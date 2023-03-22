
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

  t.pos.x = float(int(t.pos.x));
  t.pos.y = float(int(t.pos.y));
  t.size.x = float(int(t.size.x));
  t.size.y = float(int(t.size.y));

  // If not drawing top left, then we align to center
  if(!bool(t.renderOptions & RENDER_OPTION_TOP_LEFT))
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

  // 0.6 because I HATE OPENGL IT'S SHIT! -> it's not needed, removed
  if(bool(t.renderOptions & RENDER_OPTION_FLIP_X))
  {
    left = float(t.atlasOffset.x + t.spriteSize.x);
    right = float(t.atlasOffset.x);
  }
  else
  {
    left = float(t.atlasOffset.x);
    right = float(t.atlasOffset.x + t.spriteSize.x);
  }
  if(bool(t.renderOptions & RENDER_OPTION_FLIP_Y))
  {
    top = float(t.atlasOffset.y + t.spriteSize.y);
    bottom = float(t.atlasOffset.y);
  }
  else
  {
    top = float(t.atlasOffset.y);
    bottom = float(t.atlasOffset.y + t.spriteSize.y);
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