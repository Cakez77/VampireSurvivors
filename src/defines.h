#pragma once
#include <cstdint>

#define EPSILON 0.000001f

#define internal static
#define global_variable static

#define KB(x) ((uint64_t)1024 * x)
#define MB(x) ((uint64_t)1024 * KB(x))
#define GB(x) ((uint64_t)1024 * MB(x))

#define ArraySize(arr) sizeof((arr)) / sizeof((arr)[0])

#define BIT(x) (1u << (x))

// @Note(tkap, 21/11/2022) default switch case macro
#define invalid_default_case default: { CAKEZ_ASSERT(false, ""); }