#include "assets.h"
#include "colors.h"
#include "easing_functions.h"
#include "my_math.h"
#include "render_interface.h"

// Needed for Fucking rand() function, CRINGE
#include <cstdLib>

//#############################################################
//                  Internal Structures
//#############################################################
struct Entity
{
  int ID;
  SpriteID spriteID = SPRITE_ENEMY_01;
  Vec2 pos;
  Vec2 desiredDirection;
  Vec2 seperationForce;
  float pushTime;
  Vec2 pushDirection;
  float scale = UNIT_SCALE;
  Vec4 color = COLOR_WHITE;
  
  Circle collider  = {{0.0f, 0.0f}, 20.0f};
  
  int hp = 100;
  int attack = 10;
  float attackTime;
};

enum WeaponID
{
  WEAPON_WHIP,
  WEAPON_COUNT,
};

struct Weapon
{
  WeaponID ID;
  int level;
  float timePassed;
};

struct ActiveAttack
{
  WeaponID ID;
  float timePassed;
  
  Vec2 pos;
  
  union
  {
    struct
    {
      int maxSlashCount;
      int currentSlashCount;
    } whip;
  };
};

struct Player
{
  SpriteID spriteID = SPRITE_HERO_KARATE_MAN;
  Vec2 pos;
  Circle collider ={{0.0f, 0.0f}, 12.0f};
  float speed = 400.0f;
  bool flipX;
  
  int hp = 300;
  
  int weaponCount;
  Weapon weapons[WEAPON_COUNT];
};

struct DamagingArea
{
  SpriteID spriteID;
  float timePassed;
  float duration;
  Vec2 pos;
  Vec2 size;
  
  int hitEnemiesCount;
  int hitEnemyIDS[MAX_ENEMIES];
};

struct GameState
{
  int entityIDCounter = 1;
  float totalTime;
  float spawnTimer;
  
  int currentSpawnBreakPoint;
  float spawnsPerSecond;
  int spawnCounter;
  
  uint32_t enemyCount;
  Entity enemies[MAX_ENEMIES];
  
  int activeAttacksCount;  
  ActiveAttack activeAttacks[MAX_ACTIVE_WEAPONS];
  
  int damagingAreasCount;  
  DamagingArea damagingAreas[MAX_DAMAGING_AREAS];
  
  Player player;
  float playerScreenEdgeDist;
};


//#############################################################
//                  Global Variables
//#############################################################
global_variable GameState gameState = {};


//#############################################################
//                  Internal Functions
//#############################################################
internal void player_add_weapon(WeaponID ID, int level = 0)
{
  Weapon w = {};
  w.ID = ID;
  w.level = level;
  
  // TODO: We can't add inifite atcive weapons, some HAVE to be passive effects
  gameState.player.weapons[gameState.player.weaponCount++] = w;
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
  for(int hitIdx = 0; hitIdx < da.hitEnemiesCount; hitIdx++)
  {
    if(da.hitEnemyIDS[hitIdx] == enemyID)
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
  
  // @TODO(tkap, 21/11/2022): Bounds check. C arrays are cringe, where the templates at?
  gameState.damagingAreas[gameState.damagingAreasCount++] = da;
}

internal void init_game()
{
  gameState = {};
  
  player_add_weapon(WEAPON_WHIP);
  
  gameState.player.pos.x = input.screenSize.x / 2;
  gameState.player.pos.y = input.screenSize.y / 2;
  gameState.playerScreenEdgeDist = length(vec_2(WORLD_SIZE - WORLD_SIZE / 2)) + 50.0f;
}

internal void update_game(float dt)
{
  gameState.totalTime += dt;
  gameState.spawnTimer += dt;
  
  // Spawning System
  {
    float spawnRate = 0.08f;
    while(gameState.spawnTimer > spawnRate)
    {
      if(gameState.enemyCount < MAX_ENEMIES)
      {
        // In Radians
        float randomAngle = (float)(rand() % 360) * 3.14f / 180.0f;
        
        Vec2 playerPos = gameState.player.pos;
        Vec2 spawnDirection = {cosf(randomAngle), sinf(randomAngle)};
        Vec2 spawnPos = playerPos + spawnDirection * gameState.playerScreenEdgeDist;
        
        Entity enemy = {.ID = gameState.entityIDCounter++, .pos = spawnPos};
        enemy.hp += gameState.totalTime;
        gameState.enemies[gameState.enemyCount++] = enemy;
        
        gameState.spawnTimer -= spawnRate;
      }
      else
      {
        CAKEZ_ASSERT(0, "Reached maximum amount of Enemies");
      }
    }
  }
  
  for(int enemyIdx = 0; enemyIdx < gameState.enemyCount; enemyIdx++)
  {
    Entity* enemy = &gameState.enemies[enemyIdx];
    
    float attackDelay = 0.5f;
    enemy->attackTime = min(enemy->attackTime + dt, attackDelay);
    
    // Check if colliding with player
    {
      Circle collider = get_collider(*enemy);
      Circle playerCollider = get_collider(gameState.player);
      if(circle_collision(collider, playerCollider, 0))
      {
        if(enemy->attackTime >= attackDelay)
        {
          gameState.player.hp -= enemy->attack;
          enemy->attackTime = 0.0f;
          
          if(gameState.player.hp <= 0)
          {
            init_game();
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
      for(int weaponIdx = 0; weaponIdx < gameState.player.weaponCount; weaponIdx++)
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
            aa.pos = gameState.player.pos; // TODO: Needed???
            aa.whip.maxSlashCount = w->level < 2? 3: w->level < 5? 4: 5;
            
            break;
          }
        }
        
        while(w->timePassed > skillCooldown)
        {
          w->timePassed -= skillCooldown;
          
          // Add active weapon
          gameState.activeAttacks[gameState.activeAttacksCount++] = aa;
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
  
  // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		ACTIVE ATTACKS START		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
  {
    for(int aaIdx = 0; aaIdx < gameState.activeAttacksCount; aaIdx++)
    {
      ActiveAttack* aa = &gameState.activeAttacks[aaIdx];
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
            add_damaging_area(SPRITE_EFFECT_WHIP, 
                              aa->pos + offsets[whip->currentSlashCount++], vec_2(2.0f), 0.25f); 
          }
          
          break;
        } 
        
        invalid_default_case;
      }
      
      aa->timePassed += dt;
      
      // Active Attack ran out
      if(aa->timePassed >= skillDuration) 
      { 
        *aa = gameState.activeAttacks[--gameState.activeAttacksCount];
        aaIdx--;
      }
    }
  }
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		ACTIVE ATTACKS END		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  
  
  // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		HANDLE DAMAGING AREAS START		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
  {
    for(int daIdx = 0; daIdx < gameState.damagingAreasCount; daIdx++)
    {
      DamagingArea* da = &gameState.damagingAreas[daIdx];
      
      float percentDone = da->timePassed / da->duration;
      Vec2 scale = da->size * percentDone;
      
      draw_sprite(da->spriteID, da->pos, scale, COLOR_WHITE);
      
      da->timePassed += dt;
      if(da->timePassed > da->duration)
      {
        *da = gameState.damagingAreas[--gameState.damagingAreasCount];
        daIdx--;
        continue;
      }
      
      Vec2 spriteSize = vec_2(get_sprite(da->spriteID).subSize);
      Vec2 size = scale * spriteSize;
      
      // @TODO(tkap, 21/11/2022): Scuffed pushing thing
      Rect daCollider = {da->pos, size};
      
      // TODO: Only the first one is blue!
      //draw_quad(daCollider.pos, daCollider.size, COLOR_BLUE);
      
      for(int enemyIdx = 0; enemyIdx < gameState.enemyCount; enemyIdx++)
      {
        Entity* enemy = &gameState.enemies[enemyIdx];
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
            *enemy = gameState.enemies[--gameState.enemyCount];
            enemyIdx--;
            continue;
          }
          
          // Push away
          enemy->pushTime = 1.0f;
          Vec2 pushDir = normalize(enemy->pos - da->pos);
          enemy->pushDirection = pushDir * 10.0f;
          
          // Add to hit targets
          da->hitEnemyIDS[da->hitEnemiesCount++] = enemy->ID;
        }
      }
      
    }
  }
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		HANDLE DAMAGING AREAS END		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  
}
