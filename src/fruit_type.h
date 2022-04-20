#ifndef __FRUIT_TYPE_H__
#define __FRUIT_TYPE_H__

#include <stdbool.h>

typedef struct fruit {
  char name;
  char image;
  int rad;
  int x;
  int y;
  int x_speed;
  int y_speed;
  bool throw;
  int t;
  bool hit;
  struct fruit * next;
} Fruit;

#endif
