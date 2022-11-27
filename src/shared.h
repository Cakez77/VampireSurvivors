#pragma once
#include "gameData.h"
#include "render_interface.h"

extern "C"
{
  __declspec(dllexport) void init_game(GameState* gameState, Input* inputIn, Dunno* dunnoIn);
  __declspec(dllexport) void update_game(GameState* gameState, float dt);
}