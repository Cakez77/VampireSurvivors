
// Input
layout (location = 0) in flat int renderOptions;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 textureCoordsIn;

// Output
layout (location = 0) out vec4 fragColor;

layout (binding = 0) uniform sampler2D textureAtlas;
layout (binding = 1) uniform sampler2D fontAtlas;

void main()
{
  ivec2 textureCoords = ivec2(round(textureCoordsIn));
  vec4 glyphColor = texelFetch(fontAtlas, ivec2(textureCoordsIn), 0);
  vec4 textureColor = texelFetch(textureAtlas, textureCoords, 0);
 
  if(bool(renderOptions & RENDER_OPTION_FONT))
  {
    textureColor = vec4(color.rbg, glyphColor.r);
  }

  if(textureColor.a == 0)
  {
    discard;
  }

  fragColor = textureColor * color;
}