#pragma once
#include "defines.h"


internal float ease_out_quad(float t)
{
  if(t > 1.0f)
  {
    return 1.0f;
  }
  
  return 1.0f - (1.0f - t) * (1.0f - t);
}

internal float ease_in_quad(float t)
{
  if(t > 1.0f)
  {
    return 1.0f;
  }
  
  return t * t;
}