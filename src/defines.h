#pragma once
#include <cstdint>

#define EPSILON 0.000001f

#define internal static
#define local_persist static
#define global_variable static

#define KB(x) ((uint64_t)1024 * x)
#define MB(x) ((uint64_t)1024 * KB(x))
#define GB(x) ((uint64_t)1024 * MB(x))

#define ArraySize(arr) sizeof((arr)) / sizeof((arr)[0])

#define BIT(x) (1u << (x))
#define FOURCC(str) (uint32_t)(((uint32_t)str[0]) | ((uint32_t)str[1] << 8) \
| ((uint32_t)str[2] << 16) | ((uint32_t)str[3] << 24))

// @Note(tkap, 21/11/2022) default switch case macro
#define invalid_default_case default: { CAKEZ_ASSERT(false, ""); }