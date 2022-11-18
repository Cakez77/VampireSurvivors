#version 430 core

layout (location = 0) in flat int blackFlag;

layout (location = 0) out vec4 fragColor;

void main()
{
  fragColor = bool(blackFlag)? vec4(0.0): vec4(1.0);
}