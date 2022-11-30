#pragma once
#include "assets.h"
#include "config.h"
#include "colors.h"

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
  float garlicHitTimer;
};

enum WeaponID
{
  WEAPON_WHIP,
  WEAPON_GARLIC,
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
  float pickupTriggerRadius = 60.0f;
  float pickupRadius = 20.0f;
  bool flipX;
  
  int maxHP = 300;
  int hp = 300;
  int exp;
  
  Array<Weapon, WEAPON_COUNT> weapons;
};

struct DamagingArea
{
  SpriteID spriteID;
  float timePassed;
  float duration;
  Vec2 pos;
  Vec2 size;
  
  Array<int, MAX_ENEMIES> hitEnemyIDs;
};

enum PickupType
{
  PICKUP_TYPE_EXP,
  PICKUP_TYPE_BOX
};

struct Pickup
{
  bool triggered;
  PickupType type;
  Vec2 pos;
  Vec2 vel;
};

struct GameState
{
  bool initialized = false;
  
  int entityIDCounter = 1;
  float totalTime;
  float spawnTimer;
  
  int currentSpawnBreakPoint;
  float spawnsPerSecond;
  int spawnCounter;
  
  Array<Entity, MAX_ENEMIES> enemies;
  Array<ActiveAttack, MAX_ACTIVE_ATTACKS> activeAttacks;
  Array<DamagingArea, MAX_DAMAGING_AREAS> damagingAreas;
  Array<Pickup, MAX_PICKUPS> pickups;
  
  Player player;
  float playerScreenEdgeDist;
};
