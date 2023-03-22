
// Input
layout (location = 0) in flat int renderOptions;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 textureCoordsIn;

// Output
layout (location = 0) out vec4 fragColor;

layout (binding = 0) uniform sampler2D textureAtlas;
layout (binding = 1) uniform sampler2D fontAtlas;

void main()
{
  // Don't round here - this was the actual problem
  // You already did not round the font texcoords funnily enough, which was correct
  //ivec2 textureCoords = ivec2(round(textureCoordsIn));

  vec4 color = inColor;
  vec4 glyphColor = texelFetch(fontAtlas, ivec2(textureCoordsIn), 0);
  vec4 textureColor = texelFetch(textureAtlas, ivec2(textureCoordsIn), 0);

 
  if(bool(renderOptions & RENDER_OPTION_FONT))
  {
    textureColor = vec4(color.rbg, glyphColor.r);
  }

  if(bool(renderOptions & RENDER_OPTION_GRAYSCALE_STUFF_DUNNO))
  {
    vec2 uv = textureCoordsIn / textureSize(textureAtlas, 0);
    textureColor = texture(textureAtlas, uv);

    textureColor.rgb *= textureColor.a;
    textureColor.rgb *= textureColor.a;
  }

  if(textureColor.a == 0)
  {
    discard;
  }

  fragColor = textureColor * color;
}