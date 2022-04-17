#ifndef __FRUIT_H__
#define __FRUIT_H__

#include "swipe.h"

typedef struct{
    Point center;
    char type;
} Fruit;

int isCut(Fruit fr, Point swipe);

#endif
