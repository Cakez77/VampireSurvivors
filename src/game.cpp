#include "assets.h"
#include "colors.h"
#include "my_math.h"
#include "render_interface.h"

// @Note(tkap, 21/11/2022): IDK where to put this
#define invalid_default_case default: { CAKEZ_ASSERT(false, ""); }


enum WeaponID
{
  WEAPON_WHIP,
  WEAPON_COUNT,
};

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
  
  uint32_t enemyCount;
  Entity enemies[MAX_ENEMIES];
  
  int activeWeaponsCount;  
  ActiveWeapon activeWeapons[MAX_ACTIVE_WEAPONS];
  
  int damagingAreasCount;  
  DamagingArea damagingAreas[MAX_DAMAGING_AREAS];
  
  Player player;
};

global_variable GameState gameState = {};

internal void add_damaging_area(Vec2 pos, Vec2 size, float duration);

internal void init_game()
{
  gameState.player.unlockedWeapons[WEAPON_WHIP] = true;
  
  gameState.player.pos.x = input.screenSize.x / 2;
  gameState.player.pos.y = input.screenSize.y / 2;
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
      Sprite s = get_sprite(p->spriteID);
      draw_quad({ .spriteID = p->spriteID, .pos = p->pos, 
                  .size = vec_2(s.subSize) * (3 + sinf2(gameState.totalTime * 10) * 0.25f),
                  .color = COLOR_WHITE,
                  .renderOptions = p->flipX ? RENDER_OPTION_FLIP_X : 0});
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
            add_damaging_area(aw->pos + offsets[data->index], {20, 5}, 0.5f); 
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
      Sprite s = get_sprite(SPRITE_ENEMY_01);
      draw_quad({ .spriteID = SPRITE_ENEMY_01, .pos = da->pos, 
                  .size = vec_2(s.subSize) * da->size * percent_left,
                  .color = COLOR_WHITE});
                  
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
