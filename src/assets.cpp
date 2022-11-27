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