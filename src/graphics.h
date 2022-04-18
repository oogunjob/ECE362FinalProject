#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include "lcd.h"
#include "fruit_type.h"

void pic_subset(Picture *dst, const Picture *src, int sx, int sy);
void pic_overlay(Picture *dst, int xoffset, int yoffset, const Picture *src, int transparent);
void erase3(int x, int y);
void update3(int x, int y, const Picture* img);
void erase40(int x, int y);
void update40(int x, int y, const Picture* img);
void erase50(int x, int y);
void update50(int x, int y, const Picture* img);
void erase60(int x, int y);
void update60(int x, int y, const Picture* img);
void drawCurrFruit(Fruit fruit, int radius, int prev_x, int prev_y);
void show_score(int score);

#endif
