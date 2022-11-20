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

struct GameState
{
  int waypointCount;
  Vec2 waypoints[MAX_WAYPOINTS];
  
  uint32_t towerCount;
  Entity towers[MAX_TOWERS];
  
  uint32_t enemyCount;
  Entity enemies[MAX_ENEMIES];
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
  for(uint32_t towerIdx = 0; towerIdx < gameState.towerCount; towerIdx++)
  {
    // Do something 
  }
  
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
}