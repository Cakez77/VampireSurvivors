#include "assets.h"
#include "common.h"
#include "colors.h"
#include "easing_functions.h"
#include "gameData.h"
#include "input.h"
#include "shared.h"
#include "my_math.h"

// Needed for Fucking rand() function, CRINGE
#include <cstdLib>

//#############################################################
//                  Internal Functions
//#############################################################
global_variable GameState* gameState = 0;
#include "render_interface.h"
global_variable Dunno* dunno = 0;
#include "render_interface.cpp"

internal void player_add_weapon(WeaponID ID, int level = 0)
{
  Weapon w = {};
  w.ID = ID;
  w.level = level;
  
  // TODO: We can't add inifite atcive weapons, some HAVE to be passive effects
  gameState->player.weapons.add(w);
}

internal Circle get_collider(Player p)
{
  return {p.pos + p.collider.pos, p.collider.radius};
}

internal Circle get_collider(Entity e)
{
  return {e.pos + e.collider.pos, e.collider.radius};
}

internal bool has_hit_enemy(DamagingArea da, int enemyID)
{
  bool hit = false;
  for(int hitIdx = 0; hitIdx < da.hitEnemyIDs.count; hitIdx++)
  {
    if(da.hitEnemyIDs[hitIdx] == enemyID)
    {
      hit = true;
      break;
    }
  }
  
  return hit;
}

internal void add_damaging_area(SpriteID spriteID, Vec2 pos, Vec2 size, float duration)
{
  DamagingArea da = {};
  da.spriteID = spriteID;
  da.pos = pos;
  da.size = size;
  da.duration = duration;
  
  gameState->damagingAreas.add(da);
}

__declspec(dllexport) void init_game(GameState* gameStateIn, Input* inputIn, Dunno* dunnoIn)
{
  gameState = gameStateIn;
  *gameState = {};
  input = inputIn;
  dunno = dunnoIn;
  
  player_add_weapon(WEAPON_WHIP);
  
  gameState->player.pos.x = input->screenSize.x / 2;
  gameState->player.pos.y = input->screenSize.y / 2;
  gameState->playerScreenEdgeDist = length(vec_2(WORLD_SIZE - WORLD_SIZE / 2)) + 50.0f;
}

__declspec(dllexport) void update_game(GameState* gameState, float dt)
{
  gameState->totalTime += dt;
  gameState->spawnTimer += dt;
  
  // Spawning System
  {
    float spawnRate = 0.08f;
    while(gameState->spawnTimer > spawnRate)
    {
      if(gameState->enemies.count < MAX_ENEMIES)
      {
        // In Radians
        float randomAngle = (float)(rand() % 360) * 3.14f / 180.0f;
        
        Vec2 playerPos = gameState->player.pos;
        Vec2 spawnDirection = {cosf(randomAngle), sinf(randomAngle)};
        Vec2 spawnPos = playerPos + spawnDirection * gameState->playerScreenEdgeDist;
        
        Entity enemy = {.ID = gameState->entityIDCounter++, .pos = spawnPos};
        enemy.hp += gameState->totalTime;
        gameState->enemies.add(enemy);
        
        gameState->spawnTimer -= spawnRate;
      }
      else
      {
        //CAKEZ_ASSERT(0, "Reached maximum amount of Enemies");
      }
    }
  }
  
  for(int enemyIdx = 0; enemyIdx < gameState->enemies.count; enemyIdx++)
  {
    Entity* enemy = &gameState->enemies[enemyIdx];
    
    float attackDelay = 0.5f;
    enemy->attackTime = min(enemy->attackTime + dt, attackDelay);
    
    // Check if colliding with player
    {
      Circle collider = get_collider(*enemy);
      Circle playerCollider = get_collider(gameState->player);
      if(circle_collision(collider, playerCollider, 0))
      {
        if(enemy->attackTime >= attackDelay)
        {
          gameState->player.hp -= enemy->attack;
          enemy->attackTime = 0.0f;
          
          if(gameState->player.hp <= 0)
          {
            init_game(gameState, input, dunno);
            return;
          }
        }
      }
    }
    
    enemy->pushTime -= dt;
    if(enemy->pushTime <= 0.0f)
    {
      enemy->pushDirection = {};
    }
    
    // 50 Pixels per second
    float movementDistance = 100.0f;
    
    Vec2 direction = normalize(gameState->player.pos - enemy->pos);
    enemy->desiredDirection = direction * movementDistance;
    
    // Resolve Collisions
    {
      // Reset seperation Force
      enemy->seperationForce = {};
      
      for(int neighbourIdx = 0; neighbourIdx < gameState->enemies.count; neighbourIdx++)
      {
        // Skip yourself
        if(neighbourIdx == enemyIdx)
        {
          continue;
        }
        
        Entity neighbour = gameState->enemies[neighbourIdx];
        
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
      draw_sprite(enemy->spriteID, enemy->pos, vec_2(s.subSize) * enemy->scale, {.color = enemy->color});
    }
  }
  
  // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		UPDATE PLAYER START		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
  {
    Player* p = &gameState->player;
    
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
        for(int enemyIdx = 0; enemyIdx < gameState->enemies.count; enemyIdx++)
        {
          Entity* enemy = &gameState->enemies[enemyIdx];
          
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
      for(int weaponIdx = 0; weaponIdx < gameState->player.weapons.count; weaponIdx++)
      {
        Weapon* w = &p->weapons[weaponIdx];
        w->timePassed += dt;
        
        ActiveAttack aa = {};
        float skillCooldown = 0.0f;
        
        switch(w->ID)
        {
          case WEAPON_WHIP:
          {
            skillCooldown = 1.0f;
            
            // Fill in data for Active Attack
            aa.ID = w->ID;
            aa.pos = gameState->player.pos; // TODO: Needed???
            aa.whip.maxSlashCount = w->level < 2? 3: w->level < 5? 4: 5;
            
            break;
          }
        }
        
        while(w->timePassed > skillCooldown)
        {
          w->timePassed -= skillCooldown;
          
          // Add active weapon
          gameState->activeAttacks.add(aa);
        }
      }
    }
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		PLAYER SKILLS END		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    
    // Draw Player
    {
      float playerScale = UNIT_SCALE + sinf2(gameState->totalTime * 10.0) * 0.125f;
      Sprite s = get_sprite(p->spriteID);
      draw_sprite(p->spriteID, p->pos, vec_2(s.subSize) * playerScale,
                  {.renderOptions = p->flipX ? RENDER_OPTION_FLIP_X : 0});
      // Hp Bar
      {
        // Background
        draw_quad(p->pos + Vec2{0.0f, 50.0f}, {69.0f, 10.0f}, {.color = COLOR_BLACK});
        
        draw_quad({100, 100},{20, 30});
        
        // Actual HP
        float hpPercent = (float)p->hp / (float)p->maxHP;
        draw_quad(p->pos + Vec2{-(1.0f - hpPercent) * 69.0f / 2.0f, 50.0f}, 
                  {69.0f * hpPercent, 10.0f}, {.color = COLOR_RED});
      }
    }
  }
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		UPDATE PLAYER END		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  
  // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		ACTIVE ATTACKS START		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
  {
    for(int aaIdx = 0; aaIdx < gameState->activeAttacks.count; aaIdx++)
    {
      ActiveAttack* aa = &gameState->activeAttacks[aaIdx];
      float skillDuration = 0.0f;
      
      switch(aa->ID)
      {
        case WEAPON_WHIP:
        {
          auto whip = &aa->whip;
          skillDuration = 0.25f;
          float delay = skillDuration / whip->maxSlashCount;
          
          constexpr Vec2 offsets[] = {
            {-150.0f,  -50.0f},
            { 150.0f,    0.0f},
            {-150.0f,   50.0f},
            {   0.0f,  100.0f},
            {   0.0f,   50.0f},
          };
          
          while(aa->timePassed >= whip->currentSlashCount * delay)
          {
            // Spawn Whip
            Sprite s = get_sprite(SPRITE_EFFECT_WHIP);
            add_damaging_area(SPRITE_EFFECT_WHIP, 
                              aa->pos + offsets[whip->currentSlashCount++], 
                              vec_2(s.subSize) * 2.0f, 0.25f); 
          }
          
          break;
        } 
        
        //invalid_default_case;
      }
      
      aa->timePassed += dt;
      
      // Active Attack ran out
      if(aa->timePassed >= skillDuration) 
      { 
        gameState->activeAttacks.remove_and_swap(aaIdx--);
      }
    }
  }
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		ACTIVE ATTACKS END		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  
  
  // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		HANDLE DAMAGING AREAS START		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
  {
    for(int daIdx = 0; daIdx < gameState->damagingAreas.count; daIdx++)
    {
      DamagingArea* da = &gameState->damagingAreas[daIdx];
      
      float percentDone = da->timePassed / da->duration;
      Vec2 size = da->size * percentDone;
      
      
      draw_sprite(da->spriteID, da->pos, size, 
                  {.renderOptions = da->pos.x < gameState->player.pos.x? RENDER_OPTION_FLIP_X: 0});
      
      da->timePassed += dt;
      if(da->timePassed > da->duration)
      {
        gameState->damagingAreas.remove_and_swap(daIdx--);
        continue;
      }
      
      Rect daCollider = {da->pos - size / 2.0, size};
      
      // TODO: Only the first one is blue!
      //draw_quad(daCollider.pos, daCollider.size, COLOR_BLUE);
      
      for(int enemyIdx = 0; enemyIdx < gameState->enemies.count; enemyIdx++)
      {
        Entity* enemy = &gameState->enemies[enemyIdx];
        Circle enemyCollider = get_collider(*enemy);
        
        float pushout;
        
        if(has_hit_enemy(*da, enemy->ID))
        {
          continue;
        }
        
        if(rect_circle_collision(daCollider, enemyCollider))
        {
          // Damage 
          enemy->hp -= 200;
          if(enemy->hp <= 0)
          {
            gameState->enemies.remove_and_swap(enemyIdx--);
            continue;
          }
          
          // Push away
          enemy->pushTime = 1.0f;
          Vec2 pushDir = normalize(enemy->pos - da->pos);
          enemy->pushDirection = pushDir * 10.0f;
          
          // Add to hit targets
          da->hitEnemyIDs.add(enemy->ID);
        }
      }
      
    }
  }
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		HANDLE DAMAGING AREAS END		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}
