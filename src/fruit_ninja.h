#ifndef __FRUIT_NINJA_H__
#define __FRUIT_NINJA_H__

#include <stdbool.h>
#include "lcd.h"

typedef struct fruit {
  char * name;
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


// FUNCTION DECLARATIONS HERE
void generateFruits(char* fruit);
void fruit_ninja();


void push(Fruit** head, Fruit* fruit);
Fruit* search(Fruit* head, char* name);

#endif
