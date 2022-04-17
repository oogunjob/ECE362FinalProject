#ifndef __FRUIT_NINJA_H__
#define __FRUIT_NINJA_H__

#include <stdbool.h>

typedef struct fruit {
  const char * name;
  char* image;
  int x;
  int y;
  int x_speed;
  int y_speed;
  bool throw;
  int t;
  bool hit;
  struct fruit * next;
} Fruit;

void generateFruits(const char* name);
void fruit_ninja();

void push(Fruit** head, Fruit* fruit);
Fruit* search(Fruit* head, const char* name);

#endif
