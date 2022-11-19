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
char *platform_read_file(char *path, uint32_t *fileSize);