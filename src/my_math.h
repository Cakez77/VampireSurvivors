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

struct Rect
{
  Vec2 pos;
  Vec2 size;
};

struct Circle
{
  Vec2 pos;
  float radius;
};

//#############################################################
//                  Operators
//#############################################################
internal Vec2 operator+(Vec2 a, Vec2 other)
{
  return Vec2{
    a.x + other.x,
    a.y + other.y};
}

internal Vec2 operator+(Vec2 a, float scalar)
{
  return Vec2{
    a.x + scalar,
    a.y + scalar};
}

internal Vec2& operator+=(Vec2& a, Vec2 b)
{
  a = a + b;
  return a;
}

internal Vec2 operator-(Vec2 a, Vec2 other)
{
  return Vec2{
    a.x - other.x,
    a.y - other.y};
}

internal Vec2& operator-=(Vec2& a, Vec2 b)
{
  a = a - b;
  return a;
}

internal Vec2 operator*(Vec2 a, Vec2 other)
{
  return Vec2{
    a.x * other.x,
    a.y * other.y};
}

internal Vec2 operator*(Vec2 a, float scalar)
{
  return Vec2{
    a.x * scalar,
    a.y * scalar};
}

internal Vec2 operator/(Vec2 a, Vec2 other)
{
  return Vec2{
    a.x / other.x,
    a.y / other.y};
}

internal Vec2 operator/(Vec2 a, float scalar)
{
  return Vec2{
    a.x / scalar,
    a.y / scalar};
}

internal Vec2 vec_2(float x, float y)
{
  return {x, y};
}

internal Vec2 vec_2(float val)
{
  return {val, val};
}

internal IVec2 operator+(IVec2 a, IVec2 other)
{
  return IVec2{
    a.x + other.x,
    a.y + other.y};
}

internal IVec2 operator-(IVec2 a, IVec2 other)
{
  return IVec2{
    a.x - other.x,
    a.y - other.y};
}

internal IVec2 operator/(IVec2 a, IVec2 other)
{
  return IVec2{
    a.x / other.x,
    a.y / other.y};
}

internal IVec2 operator/(IVec2 a, int scalar)
{
  return IVec2{
    a.x / scalar,
    a.y / scalar};
}

internal IVec2 ivec_2(int val)
{
  return {val, val};
}

internal bool operator==(Vec4 a, Vec4 b)
{
  return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

internal Vec4 operator*(Vec4 a, Vec4 other)
{
  return Vec4{
    a.x * other.x,
    a.y * other.y,
    a.z * other.z,
    a.w * other.w};
}

internal Vec4 operator*(Vec4 a, float scalar)
{
  return Vec4{
    a.x * scalar,
    a.y * scalar,
    a.z * scalar,
    a.w * scalar};
}

internal Vec4& operator*=(Vec4& a, Vec4 b)
{
  a = a * b;
  return a;
}

internal Vec4& operator*=(Vec4& a, float scalar)
{
  a = a * scalar;
  return a;
}

//#############################################################
//                  Functions
//#############################################################
internal float clamp(float value, float min, float max)
{
  if (value > max)
  {
    value = max;
  }
  else if (value < min)
  {
    value = min;
  }
  
  return value;
}

internal int clamp(int value, int min, int max)
{
  if (value > max)
  {
    value = max;
  }
  else if (value < min)
  {
    value = min;
  }
  
  return value;
}

internal Vec2 vec_2(IVec2 v)
{
  return {(float)v.x, (float)v.y};
}

internal float length(Vec2 v)
{
  return (float)sqrt((v.x * v.x) + (v.y * v.y));
}

internal float length_squared(Vec2 v)
{
  return (v.x * v.x) + (v.y * v.y);
}

internal Vec2 normalize(Vec2 v)
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

internal bool point_in_circle(Vec2 point, Circle c)
{
  Vec2 pointToOrigen = c.pos - point;
  float lengthSquared = length_squared(pointToOrigen);
  return lengthSquared <= c.radius * c.radius;
}

internal bool point_in_rect(Vec2 point, Rect rect)
{
  return (point.x >= rect.pos.x &&
          point.x <= rect.pos.x + rect.size.x &&
          point.y >= rect.pos.y &&
          point.y <= rect.pos.y + rect.size.y);
}

internal bool circle_collision(Circle a, Circle b, float* pushout = 0)
{
  bool result = false;
  
  float distanceSquared = length_squared(b.pos - a.pos);
  if(distanceSquared < (a.radius + b.radius) * (a.radius + b.radius))
  {
    if(pushout)
    {
      *pushout = (float)sqrt(distanceSquared) - a.radius - b.radius;
    }
    result = true;
  }
  
  return result;
}

internal bool rect_circle_collision(Rect rect, Circle c)
{
  Vec2 topLeft = rect.pos;
  Vec2 topRight = rect.pos + Vec2{rect.size.x};
  Vec2 bottomLeft = rect.pos + Vec2{0.0f, rect.size.y};
  Vec2 bottomRight = rect.pos + rect.size;
  
  Vec2 projectedPos = {};
  projectedPos.y = clamp(c.pos.y, topLeft.y, bottomRight.y);
  projectedPos.x = clamp(c.pos.x, topLeft.x, bottomRight.x);
  
  if(point_in_circle(projectedPos, c))
  {
    return true;
  }
  
  return false;
}

template <typename T>
internal T max(T a, T b)
{
  return a > b ? a : b;
}

template <typename T>
internal T min(T a, T b)
{
  return a < b ? a : b;
}

internal int min(int a, int b)
{
  return a < b? a : b;
}

internal int max(int a, int b)
{
  return a > b? a : b;
}
