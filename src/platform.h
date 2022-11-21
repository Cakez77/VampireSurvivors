#pragma once


enum TextColor
{
  TEXT_COLOR_WHITE,
  TEXT_COLOR_GREEN,
  TEXT_COLOR_YELLOW,
  TEXT_COLOR_RED,
  TEXT_COLOR_LIGHT_RED,
};

void platform_log(char *msg, TextColor color);
void platform_print_error();
char* platform_read_file(char *path, uint32_t *fileSize);
long long platform_last_edit_timestamp(char* path);

// Annoying function needed because stb_image.h allocates FUCKING memory, AND I DON'T LIKE THAT
char* platform_allocate_transient(uint32_t sizeInBytes);
