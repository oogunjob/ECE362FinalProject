#include "fruit.h"
#define MELON_RADIUS 30 //Diameter of 60
#define LEMON_RADIUS 25 //Diameter of 50
#define GRAPE_RADIUS 20 //Diameter of 40
#define APPLE_RADIUS 25 //Diameter of 50

int isCut(Fruit fr, Point swipe) {
    int radius = 0;

    switch(fr.type) {
        case 'm' : radius = MELON_RADIUS;
                            break;
        case 'l' : radius = LEMON_RADIUS;
                            break;
        case 'g' : radius = GRAPE_RADIUS;
                            break;
        default  : radius = APPLE_RADIUS;
    }

    int distSwipeX = (fr.center.x - swipe.x) * (fr.center.x - swipe.x);
    int distSwipeY = (fr.center.y - swipe.y) * (fr.center.y - swipe.y);
    int distRad = radius*radius;

    return distSwipeX < distRad ? distSwipeY < distRad ? 1 : 0 : 0;
}
