#include "assets.h"
#include "common.h"
#include "colors.h"
#include "easing_functions.h"
#include "gameData.h"
#include "input.h"
#include "shared.h"
#include "texts.h"
#include "my_math.h"
#include "sound.h"

// Needed for Fucking rand() function, CRINGE
#include <cstdLib>

//#############################################################
//                  Internal Functions
//#############################################################
global_variable GameState* gameState = 0;
#include "render_interface.h"
global_variable RenderData* renderData = 0;
#include "render_interface.cpp"

internal Vec2 get_screen_pos(Vec2 pos)
{
  Vec2 screenMiddle = vec_2(SCREEN_SIZE) / 2.0f;
  Vec2 screenPos = screenMiddle + pos - gameState->player.pos;
  
  return screenPos;
}

internal void spawn_enemy(EnemyType type, bool boss = false)
{
  Entity e = {};
  
  float randomAngle = (float)(rand() % 360) * 3.14f / 180.0f;
  
  Vec2 playerPos = gameState->player.pos;
  Vec2 spawnDirection = {cosf(randomAngle), sinf(randomAngle)};
  Vec2 spawnPos = playerPos + spawnDirection * gameState->playerScreenEdgeDist;
  
  e.boss = boss;
  e.pos = spawnPos;
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
      e.spriteID = SPRITE_ENEMY_BAT;
      e.hp = 8;
      e.attack = 10;
      e.moveSpeed = 150.0f;
      
      break;
    }
    
    case ENEMY_TYPE_BAT_BOSS:
    {
      e.spriteID = SPRITE_ENEMY_BAT_BOSS;
      e.hp = 80;
      e.attack = 20;
      e.moveSpeed = 100.0f;
      
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
    
    case ENEMY_TYPE_PLANT_BOSS:
    {
      e.spriteID = SPRITE_ENEMY_PLANT_BOSS;
      e.hp = 200;
      e.attack = 10;
      e.moveSpeed = 50.0f;
      
      break;
    }
    
    case ENEMY_TYPE_MARIO_PLANT:
    {
      e.spriteID = SPRITE_ENEMY_MARIO_PLANT;
      e.hp = 40;
      e.attack = 10;
      e.moveSpeed = 25.0f;
      
      break;
    }
    
    case ENEMY_TYPE_HORNET:
    {
      e.spriteID = SPRITE_ENEMY_HORNET;
      e.hp = 10;
      e.attack = 10;
      e.moveSpeed = 200.0f;
      
      break;
    }
    
    case ENEMY_TYPE_HORNET_BOSS:
    {
      e.spriteID = SPRITE_ENEMY_HORNET_BOSS;
      e.hp = 400;
      e.attack = 10;
      e.moveSpeed = 200.0f;
      
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

internal Vec2 get_chunk_offset(Vec2 pos)
{
  Vec2 tileSize = Vec2{64.0f, 64.0f} * UNIT_SCALE;
  float chunkWidth = (float)MAP_CHUNK_TILE_COUNT * tileSize.x;
  float chunkOffsetX = fmodf(pos.x, chunkWidth);
  float chunkOffsetY = fmodf(pos.y, chunkWidth);
  
  // Loop inside the Chunk
  {
    if(chunkOffsetX < 0.0f)
    {
      chunkOffsetX += chunkWidth;
    }
    
    if(chunkOffsetY < 0.0f)
    {
      chunkOffsetY += chunkWidth;
    }
  }
  
  return {chunkOffsetX, chunkOffsetY};
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
  
  DamageNumber dn = {};
  dn.pos = e->pos;
  dn.value = dmg;
  gameState->damageNumbers.add(dn);
  
  if(e->hp <= 0)
  {
    // Drop EXP Gem
    {
      Pickup pickup = {};
      pickup.type = PICKUP_TYPE_EXP_BLUE;
      pickup.pos = e->pos;
      gameState->pickups.add(pickup);
    }
  }
}

internal void add_damaging_area(WeaponID weaponID, SpriteID spriteID, Vec2 pos,
                                Vec2 size, int damage, float duration)
{
  DamagingArea da = {};
  da.weaponID = weaponID;
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
internal void draw_exp_bar()
{
  // Exp Bar
  {
    draw_sprite(SPRITE_EXP_BAR_LEFT, {4.0f, 16.0f}, {8.0f, 32.0f});
    draw_sprite(SPRITE_EXP_BAR_MIDDLE, {input->screenSize.x / 2.0f, 16.0f},
                {input->screenSize.x - 16.0f, 32.0f});
    draw_sprite(SPRITE_EXP_BAR_RIGHT, {input->screenSize.x - 4.0f, 16.0f}, {8.0f, 32.0f});
    
    int level = min(gameState->player.level, ArraySize(expTable) - 1);
    float expNeeded = (float)expTable[level];
    float barSizeX = (input->screenSize.x - 15.0f) * (float)gameState->player.exp / expNeeded;
    draw_sprite(SPRITE_WHITE, {barSizeX / 2.0f + 6.0f, 15.0f}, {barSizeX, 18.0f},{.color = COLOR_BLUE});
  }
}

internal void update_level(float dt);


__declspec(dllexport) void init_game(GameState* gameStateIn, Input* inputIn,
                                     RenderData* renderDataIn)
{
  gameState = gameStateIn;
  input = inputIn;
  renderData = renderDataIn;
  
  gameState->state = GAME_STATE_MAIN_MENU;
}

__declspec(dllexport) void update_game(GameState* gameStateIn, Input* inputIn,
                                       RenderData* renderDataIn, float dt)
{
  local_persist bool slowdown = false;
  local_persist bool pause = false;
  
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
  
  if(is_key_pressed(KEY_ESCAPE))
  {
    pause = !pause;
  }
  
  if(is_key_pressed(KEY_F))
  {
    slowdown = !slowdown;
  }
  
  if(pause)
  {
    dt = 0.0f;
  }
  
  if(slowdown)
  {
    dt *= 0.25f;
  }
  
  // Draw Background
  {
    // Position inside the chunk
    Vec2 tileSize = Vec2{64.0f, 64.0f} * UNIT_SCALE;
    float chunkWidth = (float)MAP_CHUNK_TILE_COUNT * tileSize.x;
    Vec2 playerPos = gameState->player.pos;
    
    // Which Tile to use
    float playerChunkOffsetX = fmodf(playerPos.x, chunkWidth);
    float playerChunkOffsetY = fmodf(playerPos.y, chunkWidth);
    
    // Loop inside the Chunk
    {
      if(playerChunkOffsetX < 0.0f)
      {
        playerChunkOffsetX += chunkWidth;
      }
      
      if(playerChunkOffsetY < 0.0f)
      {
        playerChunkOffsetY += chunkWidth;
      }
    }
    
    int chunkCol = (int)(playerChunkOffsetX / tileSize.x);
    int chunkRow = (int)(playerChunkOffsetY / tileSize.y);
    
    if(chunkRow < 0)
    {
      chunkRow = -chunkRow;
    }
    
    if(chunkCol < 0)
    {
      chunkCol = -chunkCol;
    }
    
    Vec2 screenMiddle = vec_2(SCREEN_SIZE) / 2.0f;
    
    // Tile Offset Player
    float playerTileOffsetX = fmodf(playerPos.x, tileSize.x);
    float playerTileOffsetY = fmodf(playerPos.y, tileSize.y);
    
    // Loop inside the Tile
    {
      if(playerTileOffsetX < 0.0f)
      {
        playerTileOffsetX += tileSize.x;
      }
      
      if(playerTileOffsetY < 0.0f)
      {
        playerTileOffsetY += tileSize.y;
      }
    }
    
    Vec2 halfTileSize = tileSize / 2.0f;
    for(int tileColIdx = -13; tileColIdx <= 13; tileColIdx++)
    {
      for(int tileRowIdx = -8; tileRowIdx <= 8; tileRowIdx++)
      {
        Vec2 tilePos =
          screenMiddle -
          Vec2{playerTileOffsetX, playerTileOffsetY} -
          halfTileSize;
        
        tilePos.x += (float)tileColIdx * tileSize.x;
        tilePos.y += (float)tileRowIdx * tileSize.y;
        
        int subRowIdx = chunkRow + tileRowIdx;
        if(subRowIdx < 0)
        {
          subRowIdx += MAP_CHUNK_TILE_COUNT;
        }
        
        if(subRowIdx >= MAP_CHUNK_TILE_COUNT)
        {
          subRowIdx -= MAP_CHUNK_TILE_COUNT;
        }
        
        int subColIdx = chunkCol + tileColIdx;
        if(subColIdx < 0)
        {
          subColIdx += MAP_CHUNK_TILE_COUNT;
        }
        
        if(subColIdx >= MAP_CHUNK_TILE_COUNT)
        {
          subColIdx -= MAP_CHUNK_TILE_COUNT;
        }
        
        SpriteID spriteIDTile = ChunkTiles[subRowIdx][subColIdx];
        draw_sprite(spriteIDTile, tilePos, tileSize,
                    {.renderOptions = RENDER_OPTION_TOP_LEFT});
      }
    }
    
    // Obstacles inside the Cunk
    {
      gameState->obstacles.count = 0;
      gameState->obstacles.add({SPRITE_OBSTACLE_LOG_32, {100.0f, 100.0f, 32.0f, 32.0f}});
      gameState->obstacles.add({SPRITE_OBSTACLE_LOG_32, {100.0f, 164.0f, 32.0f, 32.0f}});
      gameState->obstacles.add({SPRITE_OBSTACLE_LOG_32, {100.0f, 228.0f, 32.0f, 32.0f}});
      gameState->obstacles.add({SPRITE_OBSTACLE_LOG_32, {100.0f, 228.0f, 32.0f, 32.0f}});
      gameState->obstacles.add({SPRITE_OBSTACLE_TREE_STUMP_48_32, {600.0f, 200.0f, 48.0f, 32.0f}});
      gameState->obstacles.add({SPRITE_OBSTACLE_TREE_STUMP_48_32, {840.0f, 600.0f, 48.0f, 32.0f}});
      
      // Top
      gameState->obstacles.add({SPRITE_OBSTACLE_PILLAR_BOTTOM_32, {800.0f,  700.0f, 32.0f, 32.0f}});
      gameState->obstacles.add({SPRITE_OBSTACLE_PILLAR_BOTTOM_32, {864.0f,  700.0f, 32.0f, 32.0f}});
      gameState->obstacles.add({SPRITE_OBSTACLE_PILLAR_BOTTOM_32, {928.0f,  700.0f, 32.0f, 32.0f}});
      gameState->obstacles.add({SPRITE_OBSTACLE_PILLAR_BOTTOM_32, {992.0f,  700.0f, 32.0f, 32.0f}});
      gameState->obstacles.add({SPRITE_OBSTACLE_PILLAR_BOTTOM_32, {1056.0f, 700.0f, 32.0f, 32.0f}});
      
      // Left
      gameState->obstacles.add({SPRITE_OBSTACLE_PILLAR_BOTTOM_32, {800.0f, 764.0f, 32.0f, 32.0f}});
      gameState->obstacles.add({SPRITE_OBSTACLE_PILLAR_BOTTOM_32, {800.0f, 828.0f, 32.0f, 32.0f}});
      gameState->obstacles.add({SPRITE_OBSTACLE_PILLAR_BOTTOM_32, {800.0f, 892.0f, 32.0f, 32.0f}});
      gameState->obstacles.add({SPRITE_OBSTACLE_PILLAR_BOTTOM_32, {800.0f, 956.0f, 32.0f, 32.0f}});
      
      // Right
      gameState->obstacles.add({SPRITE_OBSTACLE_PILLAR_BOTTOM_32, {1056.0f, 764.0f, 32.0f, 32.0f}});
      gameState->obstacles.add({SPRITE_OBSTACLE_PILLAR_BOTTOM_32, {1056.0f, 828.0f, 32.0f, 32.0f}});
      gameState->obstacles.add({SPRITE_OBSTACLE_PILLAR_BOTTOM_32, {1056.0f, 892.0f, 32.0f, 32.0f}});
      gameState->obstacles.add({SPRITE_OBSTACLE_PILLAR_BOTTOM_32, {1056.0f, 956.0f, 32.0f, 32.0f}});
      
      for(int obstacleIdx = 0; obstacleIdx < gameState->obstacles.count; obstacleIdx++)
      {
        Obstacle obstacle = gameState->obstacles[obstacleIdx];
        
        Vec2 obstaclePos = {};
        obstaclePos.x = obstacle.collider.pos.x - playerChunkOffsetX + screenMiddle.x;
        obstaclePos.y = obstacle.collider.pos.y - playerChunkOffsetY + screenMiddle.y;
        
        // Draw in current chunk
        draw_sprite(obstacle.spriteID, obstaclePos, obstacle.collider.size * 2.0f,
                    {.renderOptions = RENDER_OPTION_TOP_LEFT});
        
        // Draw in left chunk
        draw_sprite(obstacle.spriteID, obstaclePos + Vec2{-chunkWidth}, obstacle.collider.size * 2.0f,
                    {.renderOptions = RENDER_OPTION_TOP_LEFT});
        
        // Draw in right chunk
        draw_sprite(obstacle.spriteID, obstaclePos + Vec2{chunkWidth}, obstacle.collider.size * 2.0f,
                    {.renderOptions = RENDER_OPTION_TOP_LEFT});
        
        // Draw in top chunk
        draw_sprite(obstacle.spriteID, obstaclePos + Vec2{0.0f, -chunkWidth},
                    obstacle.collider.size * 2.0f,
                    {.renderOptions = RENDER_OPTION_TOP_LEFT});
        
        // Draw in bottom chunk
        draw_sprite(obstacle.spriteID, obstaclePos + Vec2{0.0f, chunkWidth},
                    obstacle.collider.size * 2.0f,
                    {.renderOptions = RENDER_OPTION_TOP_LEFT});
        
        // Draw in top left chunk
        draw_sprite(obstacle.spriteID, obstaclePos + Vec2{-chunkWidth, -chunkWidth},
                    obstacle.collider.size * 2.0f,
                    {.renderOptions = RENDER_OPTION_TOP_LEFT});
        
        // Draw in top right chunk
        draw_sprite(obstacle.spriteID, obstaclePos + Vec2{chunkWidth, -chunkWidth},
                    obstacle.collider.size * 2.0f,
                    {.renderOptions = RENDER_OPTION_TOP_LEFT});
        
        // Draw in bottom left chunk
        draw_sprite(obstacle.spriteID, obstaclePos + Vec2{-chunkWidth, chunkWidth},
                    obstacle.collider.size * 2.0f,
                    {.renderOptions = RENDER_OPTION_TOP_LEFT});
        
        // Draw in bottom right chunk
        draw_sprite(obstacle.spriteID, obstaclePos + Vec2{chunkWidth, chunkWidth},
                    obstacle.collider.size * 2.0f,
                    {.renderOptions = RENDER_OPTION_TOP_LEFT});
        
      }
    }
  }
  
  switch(gameState->state)
  {
    case GAME_STATE_MAIN_MENU:
    {
      draw_sprite(SPRITE_MAIN_MENU_BACKGROUND, vec_2(input->screenSize) / 2.0f, vec_2(SCREEN_SIZE));
      
      Vec2 buttonsPos = {input->screenSize.x / 2.0f, 400.0f};
      Vec2 buttonsSize = {200.0f, 70.0f};
      
      // Play Button
      {
        SpriteID buttonSprite = SPRITE_SLICED_MENU_02;
        
        if(point_in_rect(input->mousePosScreen ,{buttonsPos - buttonsSize / 2.0f, buttonsSize}))
        {
          buttonSprite = SPRITE_SLICED_MENU_03;
          
          if(is_key_pressed(KEY_LEFT_MOUSE))
          {
            gameState->state = GAME_STATE_SELECT_HERO;
          }
        }
        
        draw_sliced_sprite(buttonSprite, buttonsPos, buttonsSize);
        draw_text("Start", buttonsPos + Vec2{-50.0f});
        buttonsPos.y += 100.0f;
      }
      
      // Quit Button
      {
        SpriteID buttonSprite = SPRITE_SLICED_MENU_02;
        
        if(point_in_rect(input->mousePosScreen ,{buttonsPos - buttonsSize / 2.0f, buttonsSize}))
        {
          buttonSprite = SPRITE_SLICED_MENU_03;
          
          if(is_key_pressed(KEY_LEFT_MOUSE))
          {
            gameState->quitApp = true;
          }
        }
        
        draw_sliced_sprite(buttonSprite, buttonsPos, buttonsSize);
        draw_text("Quit", buttonsPos + Vec2{-40.0f});
        buttonsPos.y += 100.0f;
      }
      
      break;
    }
    
    case GAME_STATE_SELECT_HERO:
    {
      //TODO Inefficient, find a better way
      *gameState = {};
      bool madeChoice = false;
      
      // Add Obsacles
      
      Vec4 boxColor = COLOR_WHITE;
      Vec2 levelUpMenuSize = {1200.0f, 600.0f};
      Vec2 levelUpMenuPos = vec_2(input->screenSize) / 2.0f;
      Vec2 rectSize = vec_2(120.0f) * 3.0f;
      Vec2 rectPos = levelUpMenuPos - Vec2{560.0f, 200.0f};
      Vec2 heroSize = vec_2(96.0f) * 3.0f;
      draw_text("Choose Hero", levelUpMenuPos + Vec2{-85.0f, - 250.0f});
      draw_sliced_sprite(SPRITE_SLICED_MENU_01, levelUpMenuPos, levelUpMenuSize);
      
      // Belmot
      {
        if(point_in_rect(input->mousePosScreen, {rectPos, rectSize}))
        {
          boxColor *= 2.0f;
          
          if(is_key_pressed(KEY_LEFT_MOUSE))
          {
            madeChoice = true;
            gameState->player.type = PLAYER_TYPE_BELMOT;
          }
        }
        draw_sliced_sprite(SPRITE_SLICED_MENU_01, rectPos + rectSize / 2.0f,
                           rectSize, {.color = boxColor});
        draw_sprite(SPRITE_PLAYER_BELMOT, rectPos + rectSize / 2.0f, heroSize);
        
        draw_text("Belmot", rectPos + Vec2{rectSize.x / 3.0f, rectSize.y + 20.0f});
        
        rectPos.x += rectSize.x + 20.0f;
      }
      
      // Gandalf
      {
        boxColor = COLOR_WHITE;
        if(point_in_rect(input->mousePosScreen, {rectPos, rectSize}))
        {
          boxColor *= 2.0f;
          
          if(is_key_pressed(KEY_LEFT_MOUSE))
          {
            madeChoice = true;
            gameState->player.type = PLAYER_TYPE_GANDALF;
          }
        }
        draw_sliced_sprite(SPRITE_SLICED_MENU_01, rectPos + rectSize / 2.0f,
                           rectSize, {.color = boxColor});
        draw_sprite(SPRITE_PLAYER_GANDALF, rectPos + rectSize / 2.0f, heroSize);
        
        draw_text("Gandalf", rectPos + Vec2{rectSize.x / 4.0f, rectSize.y + 20.0f});
        
        rectPos.x += rectSize.x + 20.0f;
      }
      
      // Whoswho
      {
        boxColor = COLOR_WHITE;
        if(point_in_rect(input->mousePosScreen, {rectPos, rectSize}))
        {
          boxColor *= 2.0f;
          
          if(is_key_pressed(KEY_LEFT_MOUSE))
          {
            madeChoice = true;
            gameState->player.type = PLAYER_TYPE_WHOSWHO;
          }
        }
        draw_sliced_sprite(SPRITE_SLICED_MENU_01, rectPos + rectSize / 2.0f,
                           rectSize, {.color = boxColor});
        draw_sprite(SPRITE_PLAYER_WHOSWHO, rectPos + rectSize / 2.0f, heroSize);
        
        draw_text("Whoswho", rectPos + Vec2{rectSize.x / 4.0f, rectSize.y + 20.0f});
        
        rectPos.x += rectSize.x + 20.0f;
      }
      
      
      if(madeChoice)
      {
        switch(gameState->player.type)
        {
          case PLAYER_TYPE_BELMOT:
          {
            player_add_weapon(WEAPON_WHIP);
            gameState->player.spriteID = SPRITE_PLAYER_BELMOT;
            break;
          }
          
          case PLAYER_TYPE_GANDALF:
          {
            gameState->player.spriteID = SPRITE_PLAYER_GANDALF;
            player_add_weapon(WEAPON_GARLIC);
            break;
          }
          
          case PLAYER_TYPE_WHOSWHO:
          {
            // TODO: Whoswho is gonna get a different starting weapon
            gameState->player.spriteID = SPRITE_PLAYER_WHOSWHO;
            player_add_weapon(WEAPON_MAGMA_RING);
            break;
          }
        }
        
        // One Tile is 64x64
        int chunkWidth = MAP_CHUNK_TILE_COUNT * 64;
        int mapWidth = MAP_CHUNK_COUNT * chunkWidth;
        
        // Place the Player in the middle of the Map, meaning
        // in the middle of a 15x15 chunk Grid,
        gameState->player.pos = vec_2(mapWidth / 2);
        gameState->playerScreenEdgeDist = length(vec_2(SCREEN_SIZE - SCREEN_SIZE / 2)) + 50.0f;
        
        
        gameState->state = GAME_STATE_RUNNING_LEVEL;
      }
      break;
    }
    
    case GAME_STATE_LEVEL_UP:
    {
      // Draw Enemies
      {
        for(int enemyIdx = 0; enemyIdx < gameState->enemies.count; enemyIdx++)
        {
          Entity enemy = gameState->enemies[enemyIdx];
          
          Sprite s = get_sprite(enemy.spriteID);
          draw_sprite(enemy.spriteID, get_screen_pos(enemy.pos),
                      vec_2(s.subSize) * enemy.scale,
                      {.color = enemy.color,
                        .renderOptions = enemy.desiredDirection.x > 0.0f? RENDER_OPTION_FLIP_X: 0});
        }
      }
      
      draw_exp_bar();
      
      Vec4 boxColor = COLOR_WHITE;
      Vec2 levelUpMenuSize = {800.0f, 600.0f};
      Vec2 levelUpMenuPos = vec_2(input->screenSize) / 2.0f;
      draw_text("Level Up", levelUpMenuPos + Vec2{-85.0f, - 250.0f});
      draw_sliced_sprite(SPRITE_SLICED_MENU_01, levelUpMenuPos, levelUpMenuSize);
      
      Vec2 contentPos = levelUpMenuPos + Vec2{-280.0f, -150.0f};
      Vec2 iconSize = vec_2(64.0f);
      
      // Whip
      Rect whipRect = {levelUpMenuPos.x - 350.0f, contentPos.y - 45.0f, 700.0f, 90.0f};
      if(point_in_rect(input->mousePosScreen, whipRect))
      {
        boxColor *= 2.0f;
      }
      draw_sliced_sprite(SPRITE_SLICED_MENU_01, {levelUpMenuPos.x, contentPos.y}, {700.0f, 90.0f},
                         {.color = boxColor});
      draw_sprite(SPRITE_ICON_WHIP, contentPos, iconSize);
      
      Weapon* whip = get_weapon(WEAPON_WHIP);
      int idx = 0;
      if(whip)
      {
        idx = min<int>(ArraySize(WHIP_LEVEL_DESCRIPTIONS) - 1, whip->level);
      }
      draw_text(WHIP_LEVEL_DESCRIPTIONS[idx], contentPos + Vec2{90.0f});
      contentPos.y += 110.0f;
      
      // AOE
      boxColor = COLOR_WHITE;
      Rect aoeRect = {levelUpMenuPos.x - 350.0f, contentPos.y - 45.0f, 700.0f, 90.0f};
      if(point_in_rect(input->mousePosScreen, aoeRect))
      {
        boxColor *= 2.0f;
      }
      draw_sliced_sprite(SPRITE_SLICED_MENU_01, {levelUpMenuPos.x, contentPos.y}, {700.0f, 90.0f},
                         {.color = boxColor});
      draw_sprite(SPRITE_ICON_CIRCLE, contentPos, iconSize);
      
      Weapon* garlic = get_weapon(WEAPON_GARLIC);
      idx = 0;
      if(garlic)
      {
        idx = min<int>(ArraySize(GARLIC_LEVEL_DESCRIPTIONS) - 1, garlic->level);
      }
      draw_text(GARLIC_LEVEL_DESCRIPTIONS[idx], contentPos + Vec2{90.0f});
      contentPos.y += 110.0f;
      
      // Magma Ring
      boxColor = COLOR_WHITE;
      Rect magmaRingRect = {levelUpMenuPos.x - 350.0f, contentPos.y - 45.0f, 700.0f, 90.0f};
      if(point_in_rect(input->mousePosScreen, magmaRingRect))
      {
        boxColor *= 2.0f;
      }
      draw_sliced_sprite(SPRITE_SLICED_MENU_01, {levelUpMenuPos.x, contentPos.y}, {700.0f, 90.0f},
                         {.color = boxColor});
      draw_sprite(SPRITE_ICON_MAGMA_RING, contentPos, iconSize);
      
      Weapon* magmaRing = get_weapon(WEAPON_MAGMA_RING);
      idx = 0;
      if(magmaRing)
      {
        idx = min<int>(ArraySize(MAGMA_RING_LEVEL_DESCRIPTIONS) - 1, magmaRing->level);
      }
      draw_text(MAGMA_RING_LEVEL_DESCRIPTIONS[idx], contentPos + Vec2{90.0f});
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
        
        if(point_in_rect(input->mousePosScreen, magmaRingRect))
        {
          weaponID = WEAPON_MAGMA_RING;
        }
        
        if(weaponID < WEAPON_COUNT)
        {
          bool weaponFound = false;
          for(int weaponIdx = 0; weaponIdx < gameState->player.weapons.count; weaponIdx++)
          {
            Weapon* w = &gameState->player.weapons[weaponIdx];
            
            if(w->ID == weaponID)
            {
              w->level++;
              weaponFound = true;
              break;
            }
          }
          
          if(!weaponFound)
          {
            player_add_weapon(weaponID);
          }
          
          gameState->state = GAME_STATE_RUNNING_LEVEL;
        }
      }
      
      break;
    }
    
    case GAME_STATE_RUNNING_LEVEL:
    {
      update_level(dt);
      
      break;
    }
    
    case GAME_STATE_WON:
    {
      draw_sprite(SPRITE_MAIN_MENU_BACKGROUND,
                  vec_2(input->screenSize) / 2.0f,
                  vec_2(SCREEN_SIZE));
      
      draw_text("Thanks for playing! You won!",
                {input->screenSize.x / 2.0f - 321.0f, 201.0f},
                COLOR_BLACK);
      draw_text("Thanks for playing! You won!",
                {input->screenSize.x / 2.0f - 320.0f, 200.0f},
                COLOR_RED);
      
      Vec2 buttonsPos = {input->screenSize.x / 2.0f, 400.0f};
      Vec2 buttonsSize = {240.0f, 70.0f};
      
      // Main Menu Button
      {
        SpriteID buttonSprite = SPRITE_SLICED_MENU_02;
        
        if(point_in_rect(input->mousePosScreen ,
                         {buttonsPos - buttonsSize / 2.0f, buttonsSize}))
        {
          buttonSprite = SPRITE_SLICED_MENU_03;
          
          if(is_key_pressed(KEY_LEFT_MOUSE))
          {
            gameState->state = GAME_STATE_MAIN_MENU;
          }
        }
        
        draw_sliced_sprite(buttonSprite, buttonsPos, buttonsSize);
        draw_text("Main Menu", buttonsPos + Vec2{-102.0f});
        buttonsPos.y += 100.0f;
      }
      
      
      break;
    }
  }
}

internal void update_level(float dt)
{
  gameState->totalTime += dt;
  gameState->spawnTimer += dt / 1.0f;
  
  // Win condition, kill all bosses
  bool bossPresent = false;
  
  struct BossSpawn
  {
    float time;
    EnemyType enemyType;
  };
  
  BossSpawn bossSpawns[]
  {
    {25.0f, ENEMY_TYPE_BAT_BOSS},
    {50.0f, ENEMY_TYPE_PLANT_BOSS},
    {120.0f, ENEMY_TYPE_HORNET_BOSS},
  };
  
  // Check if we need to spawn a Boss
  if(gameState->bossSpawnIdx < ArraySize(bossSpawns)
     && bossSpawns[gameState->bossSpawnIdx].time < gameState->totalTime)
  {
    spawn_enemy(bossSpawns[gameState->bossSpawnIdx++].enemyType, true);
  }
  
  // Spawning System
  {
    struct SpawnData
    {
      float time;
      float rate;
      EnemyType enemyType;
    };
    
    SpawnData spawnRates[] =
    {
      {0.0f, 0.2f, ENEMY_TYPE_MOLTEN_MIDGET},
      {15.0f, 0.1f, ENEMY_TYPE_BAT},
      {25.0f, 0.1f, ENEMY_TYPE_PLANT},
      {50.0f, 0.1f, ENEMY_TYPE_MARIO_PLANT},
      {90.0f, 0.05f, ENEMY_TYPE_HORNET}
    };
    
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
        spawn_enemy(enemyType);
        
        gameState->spawnTimer -= spawnRate;
      }
      else
      {
        //CAKEZ_ASSERT(0, "Reached maximum amount of Enemies");
      }
    }
  }
  
  // Partition The enemies into Chunks
  {
    // Clear all the chunks
    for(int chunkColIdx = 0; chunkColIdx < WORLD_GRID_SIZE.x; chunkColIdx++)
    {
      for(int chunkRowIdx = 0; chunkRowIdx < WORLD_GRID_SIZE.y; chunkRowIdx++)
      {
        gameState->worldGrid[chunkColIdx][chunkRowIdx].enemyIndices.clear();
      }
    }
    
    for(int enemyIdx = 0; enemyIdx < gameState->enemies.count; enemyIdx++)
    {
      Entity* enemy  = &gameState->enemies[enemyIdx];
      
      if(enemy->hp <= 0)
      {
        gameState->enemies.remove_idx_and_swap(enemyIdx--);
        continue;
      }
      
      float chunkSize = 100.0f;
      
      // The grid goes from (-200, -200) to (1800, 1100)
      Vec2 relativePos = enemy->pos - gameState->player.pos + Vec2{1000.0f, 650.0f};
      
      if(relativePos.x < 0.0f) // We are too far to the left side of the player
      {
        enemy->pos.x = gameState->player.pos.x + SCREEN_SIZE.x / 2.0f;
      }
      
      if(relativePos.x >= 2000.0f) // We are too far to the right side of the player
      {
        enemy->pos.x = gameState->player.pos.x - SCREEN_SIZE.x / 2.0f;
      }
      
      if(relativePos.y < 0.0f) // We are too far to the left side of the player
      {
        enemy->pos.y = gameState->player.pos.y + SCREEN_SIZE.y / 2.0f;
      }
      
      if(relativePos.y >= 1300.0f) // We are too far to the right side of the player
      {
        enemy->pos.y = gameState->player.pos.y - SCREEN_SIZE.y / 2.0f;
      }
      
      relativePos = enemy->pos - gameState->player.pos + Vec2{1000.0f, 650.0f};
      
      int chunkCol = (int)(relativePos.x / chunkSize);
      int chunkRow = (int)(relativePos.y / chunkSize);
      
      assert(chunkCol >= 0);
      assert(chunkCol < WORLD_GRID_SIZE.x);
      assert(chunkRow >= 0);
      assert(chunkRow < WORLD_GRID_SIZE.y);
      
      WorldChunk* wc = &gameState->worldGrid[chunkCol][chunkRow];
      wc->enemyIndices.add(enemyIdx);
    }
  }
  
  // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		UPDATE DAMAGE NUMBERS START	vvvvvvvvvvvvvvvvvvvvvvvvv
  {
    for(int damageNumberIdx = 0;
        damageNumberIdx < gameState->damageNumbers.count;
        damageNumberIdx++)
    {
      DamageNumber* dn = &gameState->damageNumbers[damageNumberIdx];
      
      char numberText[16] = {};
      sprintf(numberText, "%d", dn->value);
      
      //draw_text(numberText, get_screen_pos(dn->pos));
      
      dn->timer += dt;
      dn->pos.y -= 20.0f * dt;
      
      if(dn->timer >= 0.5f)
      {
        gameState->damageNumbers.remove_idx_and_swap(damageNumberIdx--);
        continue;
      }
    }
  }
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		UPDATE DAMAGE NUMBERS END		^^^^^^^^^^^^^^^^^^^^^^^^^
  
  // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		UPDATE PICKUPS START		vvvvvvvvvvvvvvvvvvvvvvvvv
  {
    Circle playerPickupCollider = get_pickup_collider(gameState->player);
    Circle playerPickupTriggerCollider = get_pickup_trigger_collider(gameState->player);
    
    int blueCrystalCount = 0;
    int greenCrystalCount = 0;
    for(int pickupIdx = 0; pickupIdx < gameState->pickups.count; pickupIdx++)
    {
      Pickup* p = &gameState->pickups[pickupIdx];
      
      SpriteID spriteID = SPRITE_WHITE;
      switch(p->type)
      {
        case PICKUP_TYPE_EXP_BLUE:
        case PICKUP_TYPE_EXP_GREEN:
        case PICKUP_TYPE_EXP_RED:
        {
          if(p->type == PICKUP_TYPE_EXP_BLUE && !p->triggered)
          {
            if(gameState->mergeBlueCrystals)
            {
              if(blueCrystalCount == CRYSTAL_MERGE_COUNT)
              {
                p->type = PICKUP_TYPE_EXP_GREEN;
                gameState->mergeBlueCrystals = false;
                blueCrystalCount = 0;
              }
              else
              {
                blueCrystalCount++;
                gameState->pickups.remove_idx_and_swap(pickupIdx--);
                continue;
              }
            }
            else
            {
              blueCrystalCount++;
              
              if(!p->triggered &&
                 blueCrystalCount > CRYSTAL_MERGE_COUNT + 10)
              {
                gameState->mergeBlueCrystals = true;
              }
            }
          }
          else if(p->type == PICKUP_TYPE_EXP_GREEN && !p->triggered)
          {
            if(gameState->mergeGreenCrystals)
            {
              if(greenCrystalCount == CRYSTAL_MERGE_COUNT)
              {
                p->type = PICKUP_TYPE_EXP_RED;
                gameState->mergeGreenCrystals = false;
                greenCrystalCount = 0;
              }
              else
              {
                greenCrystalCount++;
                gameState->pickups.remove_idx_and_swap(pickupIdx--);
                continue;
              }
            }
            else
            {
              greenCrystalCount++;
              
              if(!p->triggered && greenCrystalCount > CRYSTAL_MERGE_COUNT + 10)
              {
                gameState->mergeGreenCrystals = true;
              }
            }
          }
          
          if(!p->triggered && point_in_circle(p->pos, playerPickupTriggerCollider))
          {
            p->triggered = true;
            
            // @Note(tkap, 29/11/2022): Start by going away from the player, same effect as VS
            p->vel = normalize(p->pos - gameState->player.pos) * 60;
          }
          
          if(p->triggered)
          {
            float duration = 0.5f;
            Vec2 oppositeDir = normalize(p->pos - gameState->player.pos);
            Vec2 dir = normalize(gameState->player.pos - p->pos);
            
            float t = min(p->time / duration, 1.0f);
            p->vel = oppositeDir * (1.0f - t) + dir * t;
            
            // Speed
            p->pos += p->vel * dt * 500.0f;
            
            p->time += dt;
          }
          
          spriteID = p->type ==
            PICKUP_TYPE_EXP_BLUE? SPRITE_CRYSTAL_BLUE: p->type == PICKUP_TYPE_EXP_GREEN?
            SPRITE_CRYSTAL_GREEN : SPRITE_CRYSTAL_RED;
          break;
        }
      }
      
      Sprite s = get_sprite(spriteID);
      draw_sprite(spriteID, get_screen_pos(p->pos), vec_2(s.subSize) * 2.0f); // pixel drawing with texelFetch can't use size * 1.5f here (if you want to support that, you need to switch to using texture() in shader instead)
      
      if(point_in_circle(p->pos, playerPickupCollider))
      {
        
        int expCount = p->type == PICKUP_TYPE_EXP_RED?
          2500 : p->type == PICKUP_TYPE_EXP_GREEN? 50 : 1;
        
        gameState->player.exp += expCount;
        
        int level = min(gameState->player.level, ArraySize(expTable) - 1);
        int expNeeded = expTable[level];
        
        if(gameState->player.exp >= expNeeded)
        {
          gameState->player.exp -= expNeeded;
          gameState->player.level++;
          gameState->state = GAME_STATE_LEVEL_UP;
        }
        
        gameState->pickups.remove_idx_and_swap(pickupIdx--);
        
        continue;
      }
    }
  }
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		UPDATE PICKUPS END		^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  
  // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		HANDLE DAMAGING AREAS START		vvvvvvvvvvvvvvvvvvv
  {
    for(int daIdx = 0; daIdx < gameState->damagingAreas.count; daIdx++)
    {
      DamagingArea* da = &gameState->damagingAreas[daIdx];
      da->timePassed += dt;
      
      switch(da->weaponID)
      {
        case WEAPON_WHIP:
        {
          float percentDone = da->timePassed / da->duration;
          Vec2 size = da->size * percentDone;
          
          draw_sprite(da->spriteID, get_screen_pos(da->pos), size,
                      {.renderOptions = da->pos.x < gameState->player.pos.x? RENDER_OPTION_FLIP_X: 0});
          
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
            
            if(rect_circle_collision(daCollider, enemyCollider, 0))
            {
              // Damage
              inflict_damage(enemy, da->damage);
              
              if(enemy->hp <= 0)
              {
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
          
          break;
        }
        
        case WEAPON_MAGMA_RING:
        {
          draw_sprite(da->spriteID, get_screen_pos(da->pos), da->size);
          Circle puddleCollider = {da->pos, da->size.x / 2.0f};
          
          for(int enemyIdx = 0; enemyIdx < gameState->enemies.count; enemyIdx++)
          {
            Entity* enemy = &gameState->enemies[enemyIdx];
            Circle enemyCollider = get_collider(*enemy);
            
            if(circle_collision(enemyCollider, puddleCollider)
               && enemy->magmaPuddleHitTimer <= 0.0f)
            {
              inflict_damage(enemy, da->damage);
              if(enemy->hp <= 0)
              {
                continue;
              }
              enemy->magmaPuddleHitTimer = 0.5f;
            }
          }
          
          break;
        }
      }
      
      if(da->timePassed > da->duration)
      {
        gameState->damagingAreas.remove_idx_and_swap(daIdx--);
        continue;
      }
    }
  }
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		HANDLE DAMAGING AREAS END		^^^^^^^^^^^^^^^^^^^^^^
  
  // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		UPDATE ENEMIES START		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
  {
    // New Partition Update Loop
    {
      for(int chunkColIdx = 0; chunkColIdx < WORLD_GRID_SIZE.x; chunkColIdx++)
      {
        for(int chunkRowIdx = 0; chunkRowIdx < WORLD_GRID_SIZE.y; chunkRowIdx++)
        {
          WorldChunk wc = gameState->worldGrid[chunkColIdx][chunkRowIdx];
          
          Vec2 chunkOffset =
            Vec2{-200.0f + 100.0f * (float)chunkColIdx, -200 + 100.0f * (float)chunkRowIdx};
          
          Vec2 chunkWorldPos = gameState->player.pos + chunkOffset;
          Rect chunkRect = {chunkWorldPos, 100.0f, 100.0f};
          
          for(int chunkEnemyIdx = 0; chunkEnemyIdx < wc.enemyIndices.count; chunkEnemyIdx++)
          {
            int enemyIdx = wc.enemyIndices[chunkEnemyIdx];
            Entity* enemy = &gameState->enemies[enemyIdx];
            
            if(enemy->boss)
            {
              bossPresent = true;
            }
            
            float attackDelay = 0.5f;
            enemy->attackTime = min(enemy->attackTime + dt, attackDelay);
            enemy->garlicHitTimer = max(enemy->garlicHitTimer - dt, 0.0f);
            enemy->magmaPuddleHitTimer = max(enemy->magmaPuddleHitTimer - dt, 0.0f);
            
            // Check if colliding with player
            {
              Circle playerCollider = get_collider(gameState->player);
              Circle enemyCollider = get_collider(*enemy);
              
              if(circle_collision(enemyCollider, playerCollider, 0))
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
            
            float movementDistance = enemy->moveSpeed;
            Vec2 direction = normalize(gameState->player.pos - enemy->pos);
            enemy->desiredDirection = direction * movementDistance;
            
            // Resolve Collisions
            {
              // Reset seperation Force
              enemy->seperationForce = {};
              
              // Look at surrounding chunks 3x3
              for(int chunkColOffset = -1; chunkColOffset < 2; chunkColOffset++)
              {
                for(int chunkRowOffset = -1; chunkRowOffset < 2; chunkRowOffset++)
                {
                  int subChunkColIdx = clamp(chunkColIdx + chunkColOffset, 0, WORLD_GRID_SIZE.x - 1);
                  int subChunkRowIdx = clamp(chunkRowIdx + chunkRowOffset, 0, WORLD_GRID_SIZE.y - 1);
                  
                  WorldChunk subChunk = gameState->worldGrid[subChunkColIdx][subChunkRowIdx];
                  
                  for(int subEnemyIdx = 0; subEnemyIdx < subChunk.enemyIndices.count; subEnemyIdx++)
                  {
                    int neighbourIdx = subChunk.enemyIndices[subEnemyIdx];
                    
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
              }
            }
            
            // Move the Enemy
            {
              float pushForce = ease_in_quad(enemy->pushTime);
              enemy->pos += (enemy->pushDirection * pushForce * 50.0f +
                             enemy->desiredDirection +
                             enemy->seperationForce * 1000.0f) * dt;
            }
            
            // Resolve Collisions with Obstacles
            {
              for(int obstacleIdx = 0; obstacleIdx < gameState->obstacles.count; obstacleIdx++)
              {
                Obstacle obstacle = gameState->obstacles[obstacleIdx];
                obstacle.collider.size.x *= UNIT_SCALE;
                obstacle.collider.size.y *= UNIT_SCALE;
                
                Circle enemyCollider = get_collider(*enemy);
                
                // Make the Collider bigger against Obstacles
                enemyCollider.radius += 6.0f;
                
                // Get Chunk Offset
                {
                  float chunkWidth = (float)MAP_CHUNK_TILE_COUNT * 64.0f * UNIT_SCALE;
                  
                  // Which Tile to use
                  enemyCollider.pos.x = fmodf(enemy->pos.x, chunkWidth);
                  enemyCollider.pos.y = fmodf(enemy->pos.y, chunkWidth);
                }
                
                Vec2 pushoutDir = {};
                if(rect_circle_collision(obstacle.collider, enemyCollider, &pushoutDir))
                {
                  enemy->pos += pushoutDir;
                }
              }
            }
            
            // Draw
            {
              Sprite s = get_sprite(enemy->spriteID);
              draw_sprite(enemy->spriteID, get_screen_pos(enemy->pos),
                          vec_2(s.subSize) * enemy->scale,
                          {.color = enemy->color,
                            .renderOptions = enemy->desiredDirection.x > 0.0f? RENDER_OPTION_FLIP_X: 0});
            }
          }
        }
      }
    }
  }
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		UPDATE ENEMIES END		^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  
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
        Vec2 heading = p->pos + dir * dt * p->speed;
        for(int obstacleIdx = 0; obstacleIdx < gameState->obstacles.count; obstacleIdx++)
        {
          Obstacle obstacle = gameState->obstacles[obstacleIdx];
          obstacle.collider.size.x *= UNIT_SCALE;
          obstacle.collider.size.y *= UNIT_SCALE;
          
          Circle playerCollider = get_collider(*p);
          
          // Make the Collider bigger against Obstacles
          playerCollider.radius += 6.0f;
          
          // Get Chunk Offset
          {
            float chunkWidth = (float)MAP_CHUNK_TILE_COUNT * 64.0f * UNIT_SCALE;
            
            // Which Tile to use
            playerCollider.pos.x = fmodf(p->pos.x, chunkWidth);
            playerCollider.pos.y = fmodf(p->pos.y, chunkWidth);
          }
          
          Vec2 pushoutDir = {};
          if(rect_circle_collision(obstacle.collider, playerCollider, &pushoutDir))
          {
            heading += pushoutDir;
          }
          
          p->pos = heading;
        }
        
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
            draw_sprite(SPRITE_EFFECT_GARLIC, vec_2(input->screenSize) / 2.0f, vec_2(radius * 2.0f));
            
            
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
          
          case WEAPON_MAGMA_RING:
          {
            skillCooldown = 1.0f;
            
            if(gameState->enemies.count)
            {
              if(w->timePassed >= skillCooldown)
              {
                w->timePassed -= skillCooldown;
                
                float spawnRadius = 250.0f;
                int spawnCount = w->level < 3? 1 : w->level < 6? 2 : 3;
                
                for(int spawnIdx = 0; spawnIdx < spawnCount; spawnIdx++)
                {
                  float randomX = (float)(rand() % (int)(spawnRadius * 2.0f)) - spawnRadius;
                  float randomY = (float)(rand() % (int)(spawnRadius * 2.0f)) - spawnRadius;
                  
                  aa.targetPos = gameState->player.pos + Vec2{randomX, randomY};
                  aa.pos = aa.targetPos - Vec2{0.0f, 1500.0f};
                  
                  // Add active weapon
                  gameState->activeAttacks.add(aa);
                }
              }
            }
            else
            {
              w->timePassed = min(w->timePassed, skillCooldown);
            }
            
            continue;
          }
        }
        
        while(w->timePassed >= skillCooldown)
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
      draw_sprite(p->spriteID, vec_2(input->screenSize)/2.0f, vec_2(s.subSize) * playerScale,
                  {.renderOptions = p->flipX ? RENDER_OPTION_FLIP_X : 0});
      // Hp Bar
      {
        Vec2 barPos = vec_2(input->screenSize) / 2.0f;
        
        // Background
        draw_quad(barPos + Vec2{0.0f, 50.0f}, {69.0f, 10.0f}, {.color = COLOR_BLACK});
        
        // Actual HP
        float hpPercent = (float)p->hp / (float)p->maxHP;
        draw_quad(barPos + Vec2{-(1.0f - hpPercent) * 69.0f / 2.0f, 50.0f},
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
            add_damaging_area(WEAPON_WHIP, SPRITE_EFFECT_WHIP,
                              aa->pos + offsets[whip->currentSlashCount++],
                              vec_2(s.subSize) * 2.0f, aa->damage, 0.25f);
          }
          
          break;
        }
        
        case WEAPON_MAGMA_RING:
        {
          Vec2 dir = normalize(aa->targetPos - aa->pos);
          aa->pos += dir * dt * 1000.0f;
          
          Weapon* w = get_weapon(WEAPON_MAGMA_RING);
          float aoeScale = w->level < 2? 1.0f: 1.3f;
          float duration = w->level < 5? 2.5f: 3.125f;
          int damage = w->level < 4? 20 : 30;
          
          Rect collisionRect = {aa->targetPos - Vec2{10.0f, 10.0f}, 20.0f, 20.0f};
          
          if(point_in_rect(aa->pos, collisionRect))
          {
            Sprite s = get_sprite(SPRITE_EFFECT_MAGMA_PUDDLE);
            add_damaging_area(WEAPON_MAGMA_RING, SPRITE_EFFECT_MAGMA_PUDDLE, aa->targetPos,
                              vec_2(s.subSize) * UNIT_SCALE * aoeScale, damage, duration);
            
            gameState->activeAttacks.remove_idx_and_swap(aaIdx--);
          }
          else
          {
            Sprite s = get_sprite(SPRITE_EFFECT_MAGMA_BALL);
            draw_sprite(SPRITE_EFFECT_MAGMA_BALL, get_screen_pos(aa->pos),
                        vec_2(s.subSize) * UNIT_SCALE);
          }
          
          continue;
        }
        
        //invalid_default_case;
      }
      
      aa->timePassed += dt;
      
      // Active Attack ran out
      if(aa->timePassed >= skillDuration)
      {
        gameState->activeAttacks.remove_idx_and_swap(aaIdx--);
      }
    }
  }
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		ACTIVE ATTACKS END		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  
  // Game time
  {
    char buffer[32] = {};
    
    int minutes = (int)(gameState->totalTime / 60.0f);
    int seconds = (int)(fmodf(gameState->totalTime, 60));
    
    // Yes, I know this is terrible, it is how it is
    if(minutes <= 0)
    {
      if(seconds <= 9)
      {
        sprintf(buffer, "00:0%d", seconds);
      }
      else
      {
        sprintf(buffer, "00:%d", seconds);
      }
    }
    else if(minutes <= 9)
    {
      if(seconds <= 9)
      {
        sprintf(buffer, "0%d:0%d", minutes, seconds);
      }
      else
      {
        sprintf(buffer, "0%d:%d", minutes, seconds);
      }
    }
    else
    {
      if(seconds <= 9)
      {
        sprintf(buffer, "%d:0%d", minutes, seconds);
      }
      else
      {
        sprintf(buffer, "%d:%d", minutes, seconds);
      }
    }
    
    draw_text(buffer, {input->screenSize.x / 2.0f - 20.0f, 40.0f});
  }
  
  // Win condition
  if(gameState->bossSpawnIdx >= ArraySize(bossSpawns) &&
     !bossPresent)
  {
    gameState->state = GAME_STATE_WON;
  }
  
  draw_exp_bar();
}