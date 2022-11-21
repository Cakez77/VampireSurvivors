#pragma once

#include <math.h>

//#############################################################
//                  Structures
//#############################################################
struct Vec2 
{
  float x;
  float y;
};

struct IVec2
{
  int x;
  int y;
};

struct Vec4
{
  float x;
  float y;
  float z;
  float w;
};

//#############################################################
//                  Operators
//#############################################################
Vec2 operator+(Vec2 a, Vec2 other)
{
  return Vec2{
    a.x + other.x,
    a.y + other.y};
}

Vec2 operator-(Vec2 a, Vec2 other)
{
  return Vec2{
    a.x - other.x,
    a.y - other.y};
}

Vec2& operator+=(Vec2& a, Vec2 b)
{
  a = a + b;
  return a;
}


Vec2 operator*(Vec2 a, Vec2 other)
{
  return Vec2{
    a.x * other.x,
    a.y * other.y};
}

Vec2 operator*(Vec2 a, float scalar)
{
  return Vec2{
    a.x * scalar,
    a.y * scalar};
}

Vec2 operator/(Vec2 a, Vec2 other)
{
  return Vec2{
    a.x / other.x,
    a.y / other.x};
}

Vec2 operator/(Vec2 a, float scalar)
{
  return Vec2{
    a.x / scalar,
    a.y / scalar};
}

IVec2 operator-(IVec2 a, IVec2 other)
{
  return IVec2{
    a.x - other.x,
    a.y - other.x};
}

IVec2 operator/(IVec2 a, IVec2 other)
{
  return IVec2{
    a.x / other.x,
    a.y / other.x};
}

IVec2 operator/(IVec2 a, int scalar)
{
  return IVec2{
    a.x / scalar,
    a.y / scalar};
}

//#############################################################
//                  Fucntions
//#############################################################
Vec2 vec_2(IVec2 v)
{
  return {(float)v.x, (float)v.y};
}

float length(Vec2 v)
{
  return (float)sqrt((v.x * v.x) + (v.y * v.y));
}

Vec2 normalize(Vec2 v)
{
  float vecLength = length(v);
  if (vecLength <= 0)
    return Vec2{0.0f, 0.0f};
  
  return Vec2{
    v.x / vecLength,
    v.y / vecLength};
}

internal float sinf2(float t)
{
  return sinf(t) * 0.5f + 0.5f;
}