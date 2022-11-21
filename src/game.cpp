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
  Vec2 pos;
};

struct GameState
{
  
  float totalTime;
  
  int waypointCount;
  Vec2 waypoints[MAX_WAYPOINTS];
  
  uint32_t enemyCount;
  Entity enemies[MAX_ENEMIES];
  
  Player player;
};

global_variable GameState gameState = {};

internal void init_game()
{
  Entity enemy = {};
  enemy.spriteID = SPRITE_ENEMY_01;
  enemy.pos = {-100.0f, 100.0f};
  gameState.enemies[gameState.enemyCount++] = enemy;
  
  enemy.spriteID = SPRITE_ENEMY_02;
  enemy.pos = {-100.0f, 100.0f};
  gameState.enemies[gameState.enemyCount++] = enemy;
  
  gameState.waypoints[gameState.waypointCount++] = {100.0f, 100.0f};
  gameState.waypoints[gameState.waypointCount++] = {400.0f, 100.0f};
  gameState.waypoints[gameState.waypointCount++] = {400.0f, 400.0f};
  gameState.waypoints[gameState.waypointCount++] = {100.0f, 400.0f};
}


internal void update_game(float dt)
{
  gameState.totalTime += dt;
  
  for(uint32_t enemyIdx = 0; enemyIdx < gameState.enemyCount; enemyIdx++)
  {
    Entity* enemy = &gameState.enemies[enemyIdx];
    
    // 50 Pixels per second
    float movementDistance = 200.0f * dt;
    
    // Follow Waypoints
    {
      if(enemy->reachedWaypointIdx + 1 < gameState.waypointCount)
      {
        Vec2 waypoint = gameState.waypoints[enemy->reachedWaypointIdx + 1];
        
        Vec2 direction = waypoint - enemy->pos;
        Vec2 directionNormalized = normalize(direction);
        float distance = length(direction);
        
        if(distance < movementDistance)
        {
          //TODO: This will cause a Hickup, will fix later
          enemy->reachedWaypointIdx++;
        }
        else
        {
          enemy->pos += directionNormalized * movementDistance;
        }
      }
    }
    
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
    dir = normalize(dir);

    p->pos += dir * dt * 400;

    {
      Sprite s = get_sprite(SPRITE_ENEMY_01);
      draw_quad({.flipX = false, .spriteID = SPRITE_ENEMY_01, .pos = p->pos, 
      .size = vec_2(s.subSize) * (10 + sinf2(gameState.totalTime * 10) * 0.5f),
      .color = COLOR_WHITE});
    }
  }
}