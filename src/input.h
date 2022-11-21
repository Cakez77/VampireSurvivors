#pragma once 
#include "my_math.h"


global_variable constexpr int KEY_COUNT = 255;

global_variable constexpr int KEY_BACKSPACE = 0x08;
global_variable constexpr int KEY_TAB = 0x09;
global_variable constexpr int KEY_ENTER = 0x0D;
global_variable constexpr int KEY_ALT = 0x12;
global_variable constexpr int KEY_ESCAPE = 0x1B;
global_variable constexpr int KEY_SPACE = 0x20;
global_variable constexpr int KEY_END = 0x23;
global_variable constexpr int KEY_HOME = 0x24;
global_variable constexpr int KEY_LEFT = 0x25;
global_variable constexpr int KEY_UP = 0x26;
global_variable constexpr int KEY_RIGHT = 0x27;
global_variable constexpr int KEY_DOWN = 0x28;
global_variable constexpr int KEY_DELETE = 0x2E;

global_variable constexpr int KEY_0 = 0x30;
global_variable constexpr int KEY_1 = 0x31;
global_variable constexpr int KEY_2 = 0x32;
global_variable constexpr int KEY_3 = 0x33;
global_variable constexpr int KEY_4 = 0x34;
global_variable constexpr int KEY_5 = 0x35;
global_variable constexpr int KEY_6 = 0x36;
global_variable constexpr int KEY_7 = 0x37;
global_variable constexpr int KEY_8 = 0x38;
global_variable constexpr int KEY_9 = 0x39;
global_variable constexpr int KEY_A = 0x41;
global_variable constexpr int KEY_B = 0x42;
global_variable constexpr int KEY_C = 0x43;
global_variable constexpr int KEY_D = 0x44;
global_variable constexpr int KEY_E = 0x45;
global_variable constexpr int KEY_F = 0x46;
global_variable constexpr int KEY_G = 0x47;
global_variable constexpr int KEY_H = 0x48;
global_variable constexpr int KEY_I = 0x49;
global_variable constexpr int KEY_J = 0x4A;
global_variable constexpr int KEY_K = 0x4B;
global_variable constexpr int KEY_L = 0x4C;
global_variable constexpr int KEY_M = 0x4D;
global_variable constexpr int KEY_N = 0x4E;
global_variable constexpr int KEY_O = 0x4F;
global_variable constexpr int KEY_P = 0x50;
global_variable constexpr int KEY_Q = 0x51;
global_variable constexpr int KEY_R = 0x52;
global_variable constexpr int KEY_S = 0x53;
global_variable constexpr int KEY_T = 0x54;
global_variable constexpr int KEY_U = 0x55;
global_variable constexpr int KEY_V = 0x56;
global_variable constexpr int KEY_W = 0x57;
global_variable constexpr int KEY_X = 0x58;
global_variable constexpr int KEY_Y = 0x59;
global_variable constexpr int KEY_Z = 0x5A;
global_variable constexpr int KEY_ADD = 0x6B;
global_variable constexpr int KEY_SUBTRACT = 0x6D;

global_variable constexpr int KEY_F1 = 0x70;
global_variable constexpr int KEY_F2 = 0x71;
global_variable constexpr int KEY_F3 = 0x72;
global_variable constexpr int KEY_F4 = 0x73;
global_variable constexpr int KEY_F5 = 0x74;
global_variable constexpr int KEY_F6 = 0x75;
global_variable constexpr int KEY_F7 = 0x76;
global_variable constexpr int KEY_F8 = 0x77;
global_variable constexpr int KEY_F9 = 0x78;
global_variable constexpr int KEY_F10 = 0x79;
global_variable constexpr int KEY_F11 = 0x7A;
global_variable constexpr int KEY_F12 = 0x7B;

global_variable constexpr int KEY_LEFT_SHIFT = 0xA0;
global_variable constexpr int KEY_RIGHT_SHIFT = 0xA1;
global_variable constexpr int KEY_LEFT_CONTROL = 0xA2;
global_variable constexpr int KEY_RIGHT_CONTROL = 0xA3;


struct KeyState
{
  int halfTransitionCount;
  bool isDown;
};

struct Input
{
  Vec2 screenSize;
  KeyState keys[KEY_COUNT];
};

global_variable Input input;

internal bool is_key_down(int key)
{
  bool result = input.keys[key].isDown;
  return result;
}

internal bool is_key_up(int key)
{
  bool result = !input.keys[key].isDown;
  return result;
}

internal bool is_key_pressed(int key)
{
  bool result = (input.keys[key].isDown && input.keys[key].halfTransitionCount == 1) || input.keys[key].halfTransitionCount > 1;
  return result;
}

internal bool is_key_released(int key)
{
  bool result = (!input.keys[key].isDown && input.keys[key].halfTransitionCount == 1) || input.keys[key].halfTransitionCount > 1;
  return result;
}
