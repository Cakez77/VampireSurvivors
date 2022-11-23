#include "assets.h"
#include "colors.h"
#include "easing_functions.h"
#include "my_math.h"
#include "render_interface.h"

// @Note(tkap, 21/11/2022): IDK where to put this
#define invalid_default_case default: { CAKEZ_ASSERT(false, ""); }


enum WeaponID
{
  WEAPON_WHIP,
  WEAPON_COUNT,
};

// Needed for Fucking rand() function, CRINGE
#include <cstdLib>

struct SpawnBreakPoint
{
  float time;
  int spawnAmount;
};

SpawnBreakPoint spawnBreakPoints[] =
{
  {0.0f, 10}, // Between 0 and 1 Seconds, spawn 10
  {1.0f, 10}, // Between 1 and 5 Seconds, spawn 10
  {5.0f, 10},
  {10.0f, 20},
  {12.0f, 0},
};


struct Entity
{
  SpriteID spriteID = SPRITE_ENEMY_01;
  Vec2 pos;
  Vec2 desiredDirection;
  Vec2 seperationForce;
  float pushTime;
  Vec2 pushDirection;
  float scale = UNIT_SCALE;
  Vec4 color = COLOR_WHITE;
  
  Circle collider  = {{0.0f, 0.0f}, 20.0f};
  
  int hp;
  int attack;
};

struct Weapon
{
  float timePassed;
};

struct Player
{
  bool flipX;
  SpriteID spriteID = SPRITE_HERO_KARATE_MAN;
  Vec2 pos;
  float speed = 400.0f;
  
  Circle collider ={{0.0f, 0.0f}, 12.0f};
  
  bool unlockedWeapons[WEAPON_COUNT];
  Weapon weapons[WEAPON_COUNT];
};

constexpr int MAX_ACTIVE_WEAPONS = 128;
struct ActiveWeapon
{
  WeaponID id;
  float timePassed;
  Vec2 pos;
  Player player;
  
  union
  {
    struct
    {
      int index;
      float subTimePassed;
    } whip;
  };
};

constexpr int MAX_DAMAGING_AREAS = 128;
struct DamagingArea
{
  float timePassed;
  float duration;
  Vec2 pos;
  Vec2 size;
};

struct GameState
{
  float totalTime;
  
  int currentSpawnBreakPoint;
  float spawnsPerSecond;
  int spawnCounter;
  
  uint32_t enemyCount;
  Entity enemies[MAX_ENEMIES];
  
  int activeWeaponsCount;  
  ActiveWeapon activeWeapons[MAX_ACTIVE_WEAPONS];
  
  int damagingAreasCount;  
  DamagingArea damagingAreas[MAX_DAMAGING_AREAS];
  
  Player player;
  float playerScreenEdgeDist;
};

global_variable GameState gameState = {};

internal SpawnBreakPoint* get_next_spawn_break_point()
{
  SpawnBreakPoint* next = 0;
  
  if(gameState.currentSpawnBreakPoint + 1 < ArraySize(spawnBreakPoints))
  {
    next = &spawnBreakPoints[gameState.currentSpawnBreakPoint + 1];
  }
  
  return next;
}

internal Circle get_collider(Entity e)
{
  return {e.pos + e.collider.pos, e.collider.radius};
}

internal void add_damaging_area(Vec2 pos, Vec2 size, float duration);

internal void init_game()
{
  gameState.player.unlockedWeapons[WEAPON_WHIP] = true;
  
  gameState.player.pos.x = input.screenSize.x / 2;
  gameState.player.pos.y = input.screenSize.y / 2;
  gameState.playerScreenEdgeDist = length(vec_2(WORLD_SIZE - WORLD_SIZE / 2)) + 50.0f;
}


internal void update_game(float dt)
{
  gameState.totalTime += dt;
  
  // Spawning System
  {
    if(((int)(gameState.totalTime * 10.0f) % 50) == 0)
    {
      // In Radians
      float randomAngle = (float)(rand() % 360) * 3.14f / 180.0f;
      
      Vec2 playerPos = gameState.player.pos;
      Vec2 spawnDirection = {cosf(randomAngle), sinf(randomAngle)};
      Vec2 spawnPos = playerPos + spawnDirection * gameState.playerScreenEdgeDist;
      
      Entity enemy = {.pos = spawnPos};
      gameState.enemies[gameState.enemyCount++] = enemy;
    }
  }
  
  for(int enemyIdx = 0; enemyIdx < gameState.enemyCount; enemyIdx++)
  {
    Entity* enemy = &gameState.enemies[enemyIdx];
    
    enemy->pushTime -= dt;
    if(enemy->pushTime <= 0.0f)
    {
      enemy->pushDirection = {};
    }
    
    // 50 Pixels per second
    float movementDistance = 100.0f;
    
    Vec2 direction = normalize(gameState.player.pos - enemy->pos);
    enemy->desiredDirection = direction * movementDistance;
    
    // Resolve Collisions
    {
      // Reset seperation Force
      enemy->seperationForce = {};
      
      for(int neighbourIdx = 0; neighbourIdx < gameState.enemyCount; neighbourIdx++)
      {
        // Skip yourself
        if(neighbourIdx == enemyIdx)
        {
          continue;
        }
        
        Entity neighbour = gameState.enemies[neighbourIdx];
        
        Vec2 neighbourDir = enemy->pos - neighbour.pos;
        float neighbourDist = length(neighbourDir);
        
        // Are the two colliding?
        float range = neighbour.collider.radius + neighbour.collider.radius;
        if(neighbourDist < range)
        {
          Vec2 seperationDir = {};
          
          if(neighbourDist == 0.0f)
          {
            Vec2 randomDirections[] = 
            {
              {1.0f, 0.0f}, // To the right
              {-1.0f, 0.0f}, // To the Left
            };
            neighbourDist = EPSILON;
            seperationDir = randomDirections[neighbourIdx > enemyIdx];
          }
          else
          {
            seperationDir = neighbourDir / neighbourDist;
          }
          
          float seperationStrength = (range - neighbourDist) / range;
          enemy->seperationForce += seperationDir * seperationStrength;
        }
      }
    }
    
    // Move the Enemy
    {
      float pushForce = ease_in_quad(enemy->pushTime);
      enemy->pos += (enemy->pushDirection * pushForce * 50.0f + 
                     enemy->desiredDirection + 
                     enemy->seperationForce * 1000.0f) * dt;
    }
    
    // Draw
    {
      Sprite s = get_sprite(enemy->spriteID);
      draw_sprite(enemy->spriteID, enemy->pos, vec_2(enemy->scale), enemy->color);
    }
  }
  
  // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		UPDATE PLAYER START		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
  {
    Player* p = &gameState.player;
    
    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		PLAYER MOVEMENT START		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    {
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
      
      if(is_key_pressed(KEY_SPACE))
      {
        for(int enemyIdx = 0; enemyIdx < gameState.enemyCount; enemyIdx++)
        {
          Entity* enemy = &gameState.enemies[enemyIdx];
          
          Vec2 enemyDir = enemy->pos - p->pos;
          float enemyDist = length(enemyDir);
          
          if(enemyDist < 200.0f)
          {
            Vec2 enemyDirNorm = normalize(enemyDir);
            float pushFactor = (200.0f - enemyDist) / 200.0f;
            
            enemy->pushTime = 1.0f;
            enemy->pushDirection = enemyDirNorm * pushFactor * 20.0f;
          }
        }
      }
      
      if(dir.x != 0)
      {
        p->flipX = dir.x > 0;
      }
      
      float dirLength = length(dir);
      
      if(dirLength > 0.0f)
      {
        dir = dir / dirLength;
        p->pos += dir * dt * p->speed;
      }
    }
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		PLAYER MOVEMENT END		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    
    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		PLAYER SKILLS START		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    {
      for(int weapon_i = 0; weapon_i < WEAPON_COUNT; weapon_i++)
      {
        if(!p->unlockedWeapons[weapon_i]) { continue; }
        
        Weapon* w = &p->weapons[weapon_i];
        w->timePassed += dt;
        constexpr float skillCooldown = 1;
        while(w->timePassed > skillCooldown)
        {
          w->timePassed -= skillCooldown;
          
          ActiveWeapon aw = {};
          aw.player = *p;
          aw.id = (WeaponID)weapon_i;
          gameState.activeWeapons[gameState.activeWeaponsCount++] = aw;
        }
      }
    }
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		PLAYER SKILLS END		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    
    // Draw Hero
    {
      draw_sprite(p->spriteID, p->pos, vec_2(UNIT_SCALE + sinf2(gameState.totalTime * 10) * 0.125f),
                  COLOR_WHITE, p->flipX ? RENDER_OPTION_FLIP_X : 0);
    }
  }
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		UPDATE PLAYER END		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  
  // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		ACTIVE WEAPONS START		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
  {
    for(int aw_i = 0; aw_i < gameState.activeWeaponsCount; aw_i++)
    {
      ActiveWeapon* aw = &gameState.activeWeapons[aw_i];
      bool remove = false;
      bool first_frame = aw->timePassed <= 0;
      
      constexpr float skill_duration = 0.25f;
      
      switch(aw->id)
      {
        case WEAPON_WHIP:
        {
          auto data = &aw->whip;
          constexpr float subDelay = skill_duration / 3.0f;
          if(first_frame)
          {
            aw->pos = aw->player.pos;
            data->subTimePassed += subDelay - dt;
            // aw->dir
          }
          data->subTimePassed += dt;
          while(data->subTimePassed >= subDelay)
          {
            data->subTimePassed -= subDelay;
            constexpr Vec2 offsets[] = {
              {150, 0},
              {-150, -50},
              {0, -150},
            };
            add_damaging_area(aw->pos + offsets[data->index], {10, 2.5f}, 0.5f); 
            data->index += 1;
          }
          
          // float percent_left = 1.0f - aw->timePassed / skill_duration;
        } break;
        
        invalid_default_case;
      }
      
      aw->timePassed += dt;
      if(aw->timePassed >= skill_duration) { remove = true; }
      if(remove)
      {
        gameState.activeWeaponsCount -= 1;
        gameState.activeWeapons[aw_i] = gameState.activeWeapons[gameState.activeWeaponsCount];
        aw_i -= 1;
      }
    }
  }
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		ACTIVE WEAPONS END		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  
  
  // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		HANDLE DAMAGING AREAS START		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
  {
    for(int da_i = 0; da_i < gameState.damagingAreasCount; da_i++)
    {
      DamagingArea* da = &gameState.damagingAreas[da_i];
      
      float percent_left = 1.0f - da->timePassed / da->duration;
      draw_sprite(SPRITE_ENEMY_01, da->pos, da->size * percent_left, COLOR_WHITE);
      
      da->timePassed += dt;
      if(da->timePassed > da->duration)
      {
        gameState.damagingAreasCount -= 1;
        gameState.damagingAreas[da_i] = gameState.damagingAreas[gameState.damagingAreasCount];
        da_i -= 1;
      }
      
      // @TODO(tkap, 21/11/2022): Loop through enemies and damage them
    }
  }
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		HANDLE DAMAGING AREAS END		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  
}

internal void add_damaging_area(Vec2 pos, Vec2 size, float duration)
{
  DamagingArea da = {};
  da.pos = pos;
  da.size = size;
  da.duration = duration;
  
  // @TODO(tkap, 21/11/2022): Bounds check. C arrays are cringe, where the templates at?
  gameState.damagingAreas[gameState.damagingAreasCount++] = da;
}
