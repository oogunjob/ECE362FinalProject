#include "stm32f0xx.h"
#include "isr.h"
#include "graphics.h"
#include "oled.h"
#include "fruit_ninja.h"

#define SCREEN_HEIGHT 320
#define SCREEN_WIDTH  240
#define VECTOR_SIZE 5
#define MELON_RADIUS 30 //Diameter of 60
#define LEMON_RADIUS 25 //Diameter of 50
#define GRAPE_RADIUS 20 //Diameter of 40
#define APPLE_RADIUS 25 //Diameter of 50
#define TempPicturePtr(name,width,height) Picture name[(width)*(height)/6+2] = { {width,height,2} }

extern Point vector[VECTOR_SIZE];
extern const Picture blade; // A 3x3 background image
int meloncut = 0;
int lemoncut = 0;
int grapecut = 0;
int applecut = 0;
int count = 0;
int test_cut_melon(int is_cut);
int test_cut_lemon(int is_cut);
int test_cut_grape(int is_cut);
int test_cut_apple(int is_cut);

int test_cut_melon(int is_cut) {
    Point center = {.x = (SCREEN_WIDTH-120+MELON_RADIUS), .y = 240+MELON_RADIUS};
    Fruit fr = {.x = center.x, .y = center.y, .name = 'm', .image = 'n'};
    is_cut = isCut(fr);

    if(is_cut == 1) {
        fr.image = 'c';
        count++;
    }

    drawCurrFruit(&fr, fr.x, fr.y);
    return is_cut;
}

int test_cut_lemon(int is_cut) {
    Point center = {.x = (SCREEN_WIDTH-120+LEMON_RADIUS), .y = 180+LEMON_RADIUS};
    Fruit fr = {.x = center.x, .y = center.y, .name = 'l', .image = 'n'};
    is_cut = isCut(fr);

    if(is_cut == 1) {
        fr.image = 'c';
        count++;
    }

    drawCurrFruit(&fr, fr.x, fr.y);
    return is_cut;
}

int test_cut_grape(int is_cut) {
    Point center = {.x = (SCREEN_WIDTH-120+GRAPE_RADIUS), .y = 120+GRAPE_RADIUS};
    Fruit fr = {.x = center.x, .y = center.y, .name = 'g', .image = 'n'};
    is_cut = isCut(fr);

    if(is_cut == 1) {
        fr.image = 'c';
        count++;
    }

    drawCurrFruit(&fr, fr.x, fr.y);
    return is_cut;
}

int test_cut_apple(int is_cut) {
    Point center = {.x = (SCREEN_WIDTH-120+APPLE_RADIUS), .y = 60+APPLE_RADIUS};
    Fruit fr = {.x = center.x, .y = center.y, .name = 'a', .image = 'n'};
    is_cut = isCut(fr);

    if(is_cut == 1) {
        fr.image = 'c';
        count++;
    }

    drawCurrFruit(&fr, fr.x, fr.y);
    return is_cut;
}
