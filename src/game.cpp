#include "assets.h"
#include "common.h"
#include "colors.h"
#include "easing_functions.h"
#include "gameData.h"
#include "input.h"
#include "shared.h"
#include "texts.h"
#include "my_math.h"

// Needed for Fucking rand() function, CRINGE
#include <cstdLib>

//#############################################################
//                  Internal Functions
//#############################################################
global_variable GameState* gameState = 0;
#include "render_interface.h"
global_variable RenderData* renderData = 0;
#include "render_interface.cpp"

internal void spawn_enemy(EnemyType type, Vec2 pos)
{
  Entity e = {};
  e.pos = pos;
  e.ID = gameState->entityIDCounter++;
  
  switch(type)
  {
    case ENEMY_TYPE_MOLTEN_MIDGET:
    {
      e.spriteID = SPRITE_ENEMY_01;
      e.hp = 5;
      e.attack = 10;
      
      break;
    }
    
    case ENEMY_TYPE_BAT:
    {
      e.spriteID = SPRITE_WHITE;
      e.hp = 8;
      e.attack = 10;
      
      break;
    }
    
    case ENEMY_TYPE_PLANT:
    {
      e.spriteID = SPRITE_ENEMY_PLANT;
      e.hp = 10;
      e.attack = 10;
      e.moveSpeed = 50.0f;
      
      break;
    }
  }
  
  gameState->enemies.add(e);
}

internal void player_add_weapon(WeaponID ID, int level = 1)
{
  Weapon w = {};
  w.ID = ID;
  w.level = level;
  
  // TODO: We can't add inifite atcive weapons, some HAVE to be passive effects
  gameState->player.weapons.add(w);
}

internal Circle get_pickup_collider(Player p)
{
  return {p.pos + p.collider.pos, p.pickupRadius};
}

internal Circle get_pickup_trigger_collider(Player p)
{
  return {p.pos + p.collider.pos, p.pickupTriggerRadius};
}

internal Circle get_collider(Player p)
{
  return {p.pos + p.collider.pos, p.collider.radius};
}

internal Circle get_collider(Entity e)
{
  return {e.pos + e.collider.pos, e.collider.radius};
}

internal Weapon* get_weapon(WeaponID weaponID)
{
  Weapon* w = 0;
  
  for(int weaponIdx = 0; weaponIdx < gameState->player.weapons.count; weaponIdx++)
  {
    Weapon* weapon = &gameState->player.weapons[weaponIdx];
    
    if(weapon->ID == weaponID)
    {
      w = weapon;
      break;
    }
  }
  
  return w;
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

internal void inflict_damage(Entity* e, int dmg)
{
  e->hp -= dmg;
  
  if(e->hp <= 0)
  {
    // Drop EXP Gem
    {
      Pickup pickup = {};
      pickup.type = PICKUP_TYPE_EXP;
      pickup.pos = e->pos;
      gameState->pickups.add(pickup);
    }
  }
}

internal void add_damaging_area(SpriteID spriteID, Vec2 pos, Vec2 size, int damage, float duration)
{
  DamagingArea da = {};
  da.spriteID = spriteID;
  da.pos = pos;
  da.size = size;
  da.duration = duration;
  da.damage = damage;
  
  gameState->damagingAreas.add(da);
}


//#############################################################
//                  Define funcions
//#############################################################
internal void update_level(float dt);


__declspec(dllexport) void init_game(GameState* gameStateIn, Input* inputIn,
                                     RenderData* renderDataIn)
{
  gameState = gameStateIn;
  input = inputIn;
  renderData = renderDataIn;
  
  *gameState = {};
  
  player_add_weapon(WEAPON_WHIP);
  player_add_weapon(WEAPON_GARLIC);
  
  gameState->player.pos = vec_2(input->screenSize) / 2.0f;
  gameState->playerScreenEdgeDist = length(vec_2(WORLD_SIZE - WORLD_SIZE / 2)) + 50.0f;
  
  gameState->state = GAME_STATE_RUNNING_LEVEL;
}

__declspec(dllexport) void update_game(GameState* gameStateIn, Input* inputIn, 
                                       RenderData* renderDataIn, float dt)
{
  // Make sure we use the correct memory
  {
    if(gameState != gameStateIn ||
       input != inputIn ||
       renderData != renderDataIn)
    {
      gameState = gameStateIn;
      input = inputIn;
      renderData = renderDataIn;
    }
  }
  
  // Draw Background
  {
    //WORLD SIZE {1600, 900};
    //1600 / 128 
    // Random Start Pos that is outside the Screen
    float startX = -13.0f;
    Vec2 tilePos = {startX, -13.0f};
    Vec2 tileSize = Vec2{64.0f, 64.0f} * 2.0f;
    for(int rowIdx = 0; rowIdx < 8; rowIdx++)
    {
      for(int colIdx = 0; colIdx < 14; colIdx++)
      {
        draw_sprite(SPRITE_TILE_GRASS_01, tilePos, tileSize, 
                    {.renderOptions = RENDER_OPTION_TOP_LEFT});
        tilePos.x += tileSize.x;
      }
      
      tilePos.x = startX;
      tilePos.y += tileSize.y;
    }
  }
  
  switch(gameState->state)
  {
    case GAME_STATE_LEVEL_UP:
    {
      Vec4 boxColor = COLOR_WHITE;
      Vec2 levelUpMenuSize = {800.0f, 600.0f};
      Vec2 levelUpMenuPos = vec_2(input->screenSize) / 2.0f;
      draw_text("Level Up", levelUpMenuPos + Vec2{-85.0f, - 250.0f});
      draw_sliced_sprite(SPRITE_SLICED_MENU_01, levelUpMenuPos, levelUpMenuSize);
      
      Vec2 contentPos = levelUpMenuPos + Vec2{-280.0f, -150.0f};
      Vec2 iconSize = vec_2(64.0f);
      
      // Whip
      Weapon* whip = get_weapon(WEAPON_WHIP);
      Rect whipRect = {levelUpMenuPos.x - 350.0f, contentPos.y - 45.0f, 700.0f, 90.0f};
      if(point_in_rect(input->mousePosScreen, whipRect))
      {
        boxColor *= 2.0f;
      }
      draw_sliced_sprite(SPRITE_SLICED_MENU_01, {levelUpMenuPos.x, contentPos.y}, {700.0f, 90.0f},
                         {.color = boxColor});
      draw_sprite(SPRITE_ICON_WHIP, contentPos, iconSize);
      int idx = min<int>(ArraySize(WHIP_LEVEL_DESCRIPTIONS) - 1, whip->level);
      draw_text(WHIP_LEVEL_DESCRIPTIONS[idx], contentPos + Vec2{90.0f});
      contentPos.y += 110.0f;
      
      // AOE
      boxColor = COLOR_WHITE;
      Weapon* garlic = get_weapon(WEAPON_GARLIC);
      Rect aoeRect = {levelUpMenuPos.x - 350.0f, contentPos.y - 45.0f, 700.0f, 90.0f};
      if(point_in_rect(input->mousePosScreen, aoeRect))
      {
        boxColor *= 2.0f;
      }
      draw_sliced_sprite(SPRITE_SLICED_MENU_01, {levelUpMenuPos.x, contentPos.y}, {700.0f, 90.0f},
                         {.color = boxColor});
      draw_sprite(SPRITE_ICON_CIRCLE, contentPos, iconSize);
      idx = min<int>(ArraySize(GARLIC_LEVEL_DESCRIPTIONS) - 1, garlic->level);
      draw_text(GARLIC_LEVEL_DESCRIPTIONS[idx], contentPos + Vec2{90.0f});
      contentPos.y += 110.0f;
      
      if(is_key_pressed(KEY_LEFT_MOUSE))
      {
        WeaponID weaponID = WEAPON_COUNT;
        
        if(point_in_rect(input->mousePosScreen, whipRect))
        {
          weaponID = WEAPON_WHIP;
        }
        
        if(point_in_rect(input->mousePosScreen, aoeRect))
        {
          weaponID = WEAPON_GARLIC;
        }
        
        if(weaponID < WEAPON_COUNT)
        {
          for(int weaponIdx = 0; weaponIdx < gameState->player.weapons.count; weaponIdx++)
          {
            Weapon* w = &gameState->player.weapons[weaponIdx];
            
            if(w->ID == weaponID)
            {
              w->level++;
              gameState->state = GAME_STATE_RUNNING_LEVEL;
            }
          }
        }
      }
      
      break;
    }
    
    case GAME_STATE_RUNNING_LEVEL:
    {
      update_level(dt);
      
      break;
    }
  }
}

internal void update_level(float dt)
{
  gameState->totalTime += dt;
  gameState->spawnTimer += dt / 1.0f;
  
  struct SpawnData
  {
    float time;
    float rate;
    EnemyType enemyType;
  };
  
  SpawnData spawnRates[] =
  {
    {0.0f, 0.2f, ENEMY_TYPE_MOLTEN_MIDGET},
    {10.0f, 0.1f, ENEMY_TYPE_PLANT}
  };
  
  // Spawning System
  {
    // Get spawnRate
    {
      assert(gameState->spawnRateIdx < ArraySize(spawnRates));
      
      if(gameState->spawnRateIdx + 1 < ArraySize(spawnRates) &&
         spawnRates[gameState->spawnRateIdx + 1].time <= gameState->totalTime)
      {
        gameState->spawnRateIdx++;
      }
    }
    
    float spawnRate = spawnRates[gameState->spawnRateIdx].rate;
    EnemyType enemyType = spawnRates[gameState->spawnRateIdx].enemyType;
    
    while(gameState->spawnTimer > spawnRate)
    {
      if(gameState->enemies.count < MAX_ENEMIES)
      {
        // In Radians
        float randomAngle = (float)(rand() % 360) * 3.14f / 180.0f;
        
        Vec2 playerPos = gameState->player.pos;
        Vec2 spawnDirection = {cosf(randomAngle), sinf(randomAngle)};
        Vec2 spawnPos = playerPos + spawnDirection * gameState->playerScreenEdgeDist;
        
        spawn_enemy(enemyType, spawnPos);
        
        gameState->spawnTimer -= spawnRate;
      }
      else
      {
        //CAKEZ_ASSERT(0, "Reached maximum amount of Enemies");
      }
    }
  }
  
  // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		UPDATE PICKUPS START		vvvvvvvvvvvvvvvvvvvvvvvvv
  {
    Circle playerPickupCollider = get_pickup_collider(gameState->player);
    Circle playerPickupTriggerCollider = get_pickup_trigger_collider(gameState->player);
    for(int pickupIdx = 0; pickupIdx < gameState->pickups.count; pickupIdx++)
    {
      Pickup* p = &gameState->pickups[pickupIdx];
      
      SpriteID spriteID = SPRITE_WHITE;
      
      switch(p->type)
      {
        case PICKUP_TYPE_EXP:
        {
          if(!p->triggered && point_in_circle(p->pos, playerPickupTriggerCollider))
          {
            p->triggered = true;
            
            // @Note(tkap, 29/11/2022): Start by going away from the player, same effect as VS
            p->vel = normalize(p->pos - gameState->player.pos) * 70;
          }
          if(p->triggered)
          {
            Vec2 dir = normalize(gameState->player.pos - p->pos);
            p->vel += dir * dt * 500;
            
            // @Note(tkap, 29/11/2022): Capping the speed
            p->vel = p->vel * 0.9f;
            
            p->pos += p->vel * dt * 10;
          }
          spriteID = SPRITE_CRYSTAL;
          break;
        }
      }
      
      Sprite s = get_sprite(spriteID);
      draw_sprite(spriteID, p->pos, vec_2(s.subSize) * 1.5f);
      
      if(point_in_circle(p->pos, playerPickupCollider))
      {
        gameState->pickups.remove_and_swap(pickupIdx--);
        gameState->player.exp++;
        
        if(gameState->player.exp >= 100)
        {
          // TODO: Level up
          gameState->player.exp = 0;
          gameState->state = GAME_STATE_LEVEL_UP;
        }
        
        continue;
      }
    }
  }
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		UPDATE PICKUPS END		^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  
  for(int enemyIdx = 0; enemyIdx < gameState->enemies.count; enemyIdx++)
  {
    Entity* enemy = &gameState->enemies[enemyIdx];
    
    float attackDelay = 0.5f;
    enemy->attackTime = min(enemy->attackTime + dt, attackDelay);
    enemy->garlicHitTimer = max(enemy->garlicHitTimer - dt, 0.0f);
    
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
            init_game(gameState, input, renderData);
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
    float movementDistance = enemy->moveSpeed;
    
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
      draw_sprite(enemy->spriteID, enemy->pos, vec_2(s.subSize) * enemy->scale, {.color = enemy->color,
                    .renderOptions = enemy->desiredDirection.x > 0.0f? RENDER_OPTION_FLIP_X: 0});
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
        aa.ID = w->ID;
        float skillCooldown = 0.0f;
        
        switch(w->ID)
        {
          case WEAPON_WHIP:
          {
            skillCooldown = 1.0f;
            
            // Fill in data for Active Attack
            aa.damage = 10;
            if(w->level >= 2)
            {
              aa.damage += 5;
            }
            if(w->level >= 4)
            {
              aa.damage += 10;
            }
            
            aa.pos = gameState->player.pos; // TODO: Needed???
            aa.whip.maxSlashCount = w->level < 2? 3: w->level < 5? 4: 5;
            
            break;
          }
          
          case WEAPON_GARLIC:
          {
            float hitRate = 0.5f;
            int damage = 6 + (w->level >= 4? 6: 0);
            float radius = 75.0f;
            radius *= w->level < 2? 1.0f : w->level < 3? 1.1f: w->level < 5? 1.25f: 1.45f;
            
            Circle garlicCollider = {gameState->player.pos, radius};
            draw_sprite(SPRITE_EFFECT_GARLIC, gameState->player.pos, vec_2(radius * 2.0f));
            
            
            for(int enemyIdx = 0; enemyIdx < gameState->enemies.count; enemyIdx++)
            {
              Entity* e = &gameState->enemies[enemyIdx];
              Circle enemyCollider = get_collider(*e);
              
              if(circle_collision(enemyCollider, garlicCollider, 0))
              {
                if(e->garlicHitTimer <= 0.0f)
                {
                  inflict_damage(e, damage);
                  e->garlicHitTimer += hitRate;
                  
                  if(e->hp <= 0)
                  {
                    gameState->enemies.remove_and_swap(enemyIdx--);
                    continue;
                  }
                  
                  if(w->level >= 6)
                  {
                    //do knockback
                    e->pushTime = 1.0f;
                    Vec2 pushDir = normalize(e->pos - gameState->player.pos);
                    e->pushDirection = pushDir * 5.0f;
                  }
                }
              }
            }
            
            continue;
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
      float playerScale = UNIT_SCALE + sinf2(gameState->totalTime * 10.0f) * 0.125f;
      Sprite s = get_sprite(p->spriteID);
      draw_sprite(p->spriteID, p->pos, vec_2(s.subSize) * playerScale,
                  {.renderOptions = p->flipX ? RENDER_OPTION_FLIP_X : 0});
      // Hp Bar
      {
        // Background
        draw_quad(p->pos + Vec2{0.0f, 50.0f}, {69.0f, 10.0f}, {.color = COLOR_BLACK});
        
        // Actual HP
        float hpPercent = (float)p->hp / (float)p->maxHP;
        draw_quad(p->pos + Vec2{-(1.0f - hpPercent) * 69.0f / 2.0f, 50.0f}, 
                  {69.0f * hpPercent, 10.0f}, {.color = COLOR_RED});
      }
    }
  }
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		UPDATE PLAYER END		^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  
  
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
            { 150.0f,  100.0f},
            {-150.0f, -100.0f},
          };
          
          while(aa->timePassed >= whip->currentSlashCount * delay)
          {
            // Spawn Whip
            Sprite s = get_sprite(SPRITE_EFFECT_WHIP);
            add_damaging_area(SPRITE_EFFECT_WHIP, 
                              aa->pos + offsets[whip->currentSlashCount++], 
                              vec_2(s.subSize) * 2.0f, aa->damage, 0.25f); 
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
        
        if(has_hit_enemy(*da, enemy->ID))
        {
          continue;
        }
        
        if(rect_circle_collision(daCollider, enemyCollider))
        {
          // Damage 
          inflict_damage(enemy, da->damage);
          
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
  
  // Exp Bar
  {
    draw_sprite(SPRITE_EXP_BAR_LEFT, {4.0f, 16.0f}, {8.0f, 32.0f});
    draw_sprite(SPRITE_EXP_BAR_MIDDLE, {input->screenSize.x / 2.0f, 16.0f}, 
                {input->screenSize.x - 16.0f, 32.0f});
    draw_sprite(SPRITE_EXP_BAR_RIGHT, {input->screenSize.x - 4.0f, 16.0f}, {8.0f, 32.0f});
    
    float barSizeX = (input->screenSize.x - 15.0f) * (float)gameState->player.exp / 100.0f;
    draw_sprite(SPRITE_WHITE, {barSizeX / 2.0f + 6.0f, 15.0f}, {barSizeX, 18.0f},{.color = COLOR_BLUE});
  }
}