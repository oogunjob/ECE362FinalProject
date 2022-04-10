#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include "lcd.h"

void update2(int x, int y);
void erase(int x, int y);
void pic_subset(Picture *dst, const Picture *src, int sx, int sy);
void pic_overlay(Picture *dst, int xoffset, int yoffset, const Picture *src, int transparent);
void erase(int x, int y);
void update(int x, int y);
void update2(int x, int y);
void basic_drawing();

#endif
