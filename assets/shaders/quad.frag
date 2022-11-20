#version 430 core

layout (location = 0) in vec4 color;
layout (location = 1) in vec2 textureCoordsIn;

layout (location = 0) out vec4 fragColor;

uniform sampler2D textureAtlas;

void main()
{
  ivec2 textureCoords = ivec2(textureCoordsIn);
  vec4 textureColor = texelFetch(textureAtlas, textureCoords, 0);

  if(textureColor.a == 0)
  {
    discard;
  }

  fragColor = textureColor;
}