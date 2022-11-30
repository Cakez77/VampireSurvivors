#version 430 core

layout (location = 0) in vec4 color;
layout (location = 1) in vec2 textureCoordsIn;
layout (location = 2) in vec2 uvIn;

layout (location = 0) out vec4 fragColor;

uniform sampler2D textureAtlas;

void main()
{
  ivec2 textureCoords = ivec2(round(textureCoordsIn));
  vec4 textureColor = texelFetch(textureAtlas, textureCoords, 0);
  //vec4 textureColor = texture(textureAtlas, uvIn);

  if(textureColor.a == 0)
  {
    discard;
  }

  fragColor = textureColor * color;
}