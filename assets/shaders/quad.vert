
// Output
layout (location = 0) out vec4 color;
layout (location = 1) out vec2 textureCoordsOut;

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
  // From -0.5 to 0.5 -> 1.0 -> size is 1 unit length, Counter Clockwise
  float v_size = 1.0;
  vec2 vertices[6] = 
  {
    vec2(-v_size,  v_size), // Top Left
    vec2(-v_size, -v_size), // Bottom Left
    vec2( v_size,  v_size), // Top Right
    vec2( v_size,  v_size), // Top Right
    vec2(-v_size, -v_size), // Bottom Left
    vec2( v_size, -v_size)  // Bottom Right
  };

  Transform t = transforms[gl_InstanceID];

  // Inverts the Y- Coordinate, so that y = 0 is on the top
  t.pos.y = -t.pos.y + screenSize.y;
  // This erases wobblines but introduces micro stutter
  t.pos.x = float(int(t.pos.x));
  t.pos.y = float(int(t.pos.y));
  vec2 pos = t.pos * (2.0 / screenSize) - 1.0;
  vec2 size = t.size / screenSize;
  gl_Position = vec4(vertices[gl_VertexID] * size + pos, 0.0, 1.0);

  color = materials[t.materialIdx].color;

  // Texture Coords, with flipping
  float left;
  float right;
  float top;
  float bottom;
  
  // 0.6 because I HATE OPENGL IT'S SHIT!
  if(bool(t.renderOptions & RENDER_OPTION_FLIP_X))
  {
    left = float(t.atlasOffset.x + t.spriteSize.x) - 0.6;
    right = float(t.atlasOffset.x) + 0.6;
  }
  else
  {
    left = float(t.atlasOffset.x) + 0.6;
    right = float(t.atlasOffset.x + t.spriteSize.x) - 0.6;
  }
  if(bool(t.renderOptions & RENDER_OPTION_FLIP_Y))
  {
    top = float(t.atlasOffset.y + t.spriteSize.y) - 0.6;
    bottom = float(t.atlasOffset.y) + 0.6;
  }
  else
  {
    top = float(t.atlasOffset.y) + 0.6;
    bottom = float(t.atlasOffset.y + t.spriteSize.y) - 0.6;
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