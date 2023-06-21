#pragma once

enum TextColor
{
  TEXT_COLOR_WHITE,
  TEXT_COLOR_GREEN,
  TEXT_COLOR_YELLOW,
  TEXT_COLOR_RED,
  TEXT_COLOR_LIGHT_RED,
};

enum SoundID
{
  SOUND_BACKGROUND,
  SOUND_SCHLITZ,
  
  SOUND_COUNT
};

void platform_log(char *msg, TextColor color);
void platform_print_error();
char* platform_read_file(char *path, uint32_t *fileSize);
unsigned long platform_write_file(char *path, char *buffer, uint32_t size, bool overwrite = true);
bool platform_file_exists(char *path);
void platform_delete_file(char *path);
long long platform_last_edit_timestamp(char* path);

void platform_play_sound(SoundID soundID, bool loop = false);

char* platform_allocate_transient(uint32_t sizeInBytes);
char* platform_allocate_persistent(uint32_t sizeInBytes);
