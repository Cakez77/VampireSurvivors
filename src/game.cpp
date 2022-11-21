#include "assets.h"
#include "colors.h"
#include "my_math.h"
#include "render_interface.h"

struct Entity
{
  SpriteID spriteID;
  Vec2 pos;
  float scale = 3.0f;
  Vec4 color = COLOR_WHITE;
  
  int reachedWaypointIdx = -1;
  
  int hp;
  int attack;
};

struct Player
{
  SpriteID spriteID = SPRITE_HERO_KARATE_MAN;
  Vec2 pos;
  Vec2 dir;
  float speed = 400.0f;
};

struct GameState
{
  float totalTime;
  
  uint32_t enemyCount;
  Entity enemies[MAX_ENEMIES];
  
  Player player;
};

global_variable GameState gameState = {};

internal void init_game()
{
  // Something later?
}


internal void update_game(float dt)
{
  gameState.totalTime += dt;
  
  // Spawning System
  {
    
  }
  
  for(uint32_t enemyIdx = 0; enemyIdx < gameState.enemyCount; enemyIdx++)
  {
    Entity* enemy = &gameState.enemies[enemyIdx];
    
    // 50 Pixels per second
    float movementDistance = 200.0f * dt;
    
    // Draw Enemy 
    {
      Sprite s = get_sprite(enemy->spriteID);
      draw_quad({.spriteID = enemy->spriteID, .pos = enemy->pos, 
                  .size = vec_2(s.subSize) * enemy->scale,
                  .color = enemy->color });
    }
  }
  
  // @Note(tkap, 21/11/2022): Update player
  {
    Player* p = &gameState.player;
    
    Vec2 dir = {};
    if(is_key_down(KEY_W))
    {
      dir.y -= 1;
    }
    if(is_key_down(KEY_A))
    {
      dir.x -= 1;
    }
    if(is_key_down(KEY_S))
    {
      dir.y += 1;
    }
    if(is_key_down(KEY_D))
    {
      dir.x += 1;
    }
    float dirLength = length(dir);
    
    if(dirLength > 0.0f)
    {
      // Normalize the direction
      dir = dir / dirLength;
      p->pos += dir * dt * p->speed;
      
      // Store last direction
      p->dir = dir;
    }
    
    // Draw Hero
    {
      Sprite s = get_sprite(p->spriteID);
      draw_quad({ .spriteID = p->spriteID, .pos = p->pos, 
                  .size = vec_2(s.subSize) * (3 + sinf2(gameState.totalTime * 10) * 0.5f),
                  .color = COLOR_WHITE,
                  .renderOptions = p->dir.x > 0? RENDER_OPTION_FLIP_X : 0});
    }
  }
}