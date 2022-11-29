#pragma once
#include "defines.h"

/* TODO: This could be the macro way, could test this out
#define TEST_ARRAY(_name, _type, _N) \
struct \
{\
_type data[_N]; \
int count; \
} _name

#define TEST_array_add(_array, _value) ((_array).data[(_array).count++] = (_value))
struct Test 
{
  TEST_ARRAY(array_name, float, 10);
};

void test()
{
  Test t;
  TEST_array_add(t.array_name, 0.f);
}
*/

// @Note(tkap, 29/11/2022): Better to have a bad assert than no assert :smile:
#ifdef DEBUG
  #include <stdlib.h>
  #include <stdio.h>
  #define assert(cond) if(!(cond)) { printf("ASSERT FAILED AT %s (%i)\n", __FILE__, __LINE__); exit(1); }
#else
  #define assert(cond)
#endif // DEBUG

template <typename T, int N>
struct Array
{
  static constexpr int maxElements = N;
  
  int count = 0;
  T elements[N];
  
  T& operator[](int idx)
  {
    //CAKEZ_ASSERT(idx >= 0, "Idx negative!");
    //CAKEZ_ASSERT(idx < count, "Idx out of bounds!");
    assert(idx >= 0);
    assert(idx < count);
    return elements[idx];
  }
  
  int add(T element)
  {
    //CAKEZ_ASSERT(count < maxElements, "Array Full!");
    assert(count < maxElements);
    elements[count] = element;
    return count++;
  }
  
  void remove_and_swap(int idx)
  {
    //CAKEZ_ASSERT(idx >= 0, "Idx negative!");
    //CAKEZ_ASSERT(idx < count, "Idx out of bounds!");
    assert(idx >= 0);
    assert(idx < count);
    elements[idx] = elements[--count];
  }
};