#ifndef __FRUIT_NINJA_H__
#define __FRUIT_NINJA_H__

#include <stdbool.h>
#include "fruit_type.h"
#include "graphics.h"
#include "music.h"

void generateFruits(const char name);
void fruit_ninja();

void push(Fruit** head, Fruit* fruit);
Fruit* search(Fruit* head, const char name);
int get_fruit_radius(char name);

void TIM15_IRQHandler();
void init_tim15();

bool screenIsClear();




#endif
