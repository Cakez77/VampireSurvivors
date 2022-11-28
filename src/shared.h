#pragma once
#include "gameData.h"
#include "render_interface.h"

extern "C"
{
  __declspec(dllexport) void init_game(GameState* gameStateIn, Input* inputIn,
                                       RenderData* renderDataIn);
  __declspec(dllexport) void update_game(GameState* gameStateIn, Input* inputIn, 
                                         RenderData* renderDataIn, float dt);
}