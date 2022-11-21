#include "assets.h"

#ifdef DEBUG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

internal char* TEXTURE_PATHS[] = 
{
  "assets/textures/TEXTURE_ATLAS_01.png"
};

static_assert(ArraySize(TEXTURE_PATHS) == TEXTURE_COUNT);

Sprite get_sprite(SpriteID spriteID)
{
  Sprite s = {};
  switch(spriteID)
  {
    case SPRITE_WHITE:
    {
      s.atlasOffset = {0, 0};
      s.size = {1, 1};
      s.subSize = {1, 1};
      
      break;
    }
    
    case SPRITE_ENEMY_01:
    {
      s.atlasOffset = {0, 16};
      s.size = {32, 32};
      s.subSize = {32, 32};
      
      break;
    }
    
    case SPRITE_ENEMY_02:
    {
      s.atlasOffset = {32, 0};
      s.size = {64, 64};
      s.subSize = {64, 64};
      
      break;
    }
    
    default: 
    {
      CAKEZ_ASSERT(0, "Unrecognized spriteID: %d", spriteID);
    }
  }
  
  return s;
}


char* get_asset(TextureID textureID, int* width, int* height)
{
  char* data = 0;
  
#ifdef DEBUG
  int nrChannels;
  char* stbiBullshit = (char*)stbi_load(TEXTURE_PATHS[textureID], width, height, &nrChannels, 4); 
  int textureSizeInBytes = 4 * *width * *height;
  
  
  data = platform_allocate_transient(textureSizeInBytes);
  memcpy(data, stbiBullshit, textureSizeInBytes);
  stbi_image_free(stbiBullshit);
  
#else
  CAKEZ_ASSERT(0, "Implement Assets pack file");
#endif
  
  return data;
}

long long get_last_edit_timestamp(TextureID textureID)
{
  return platform_last_edit_timestamp(TEXTURE_PATHS[textureID]);
}