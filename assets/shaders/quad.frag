#version 430 core

layout (location = 0) in flat vec4 color;

layout (location = 0) out vec4 fragColor;

void main()
{
  fragColor = color;
}