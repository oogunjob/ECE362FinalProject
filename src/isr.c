#include "stm32f0xx.h"
#include "isr.h"
#include "graphics.h"
#include "oled.h"
#define SCREEN_HEIGHT 320
#define SCREEN_WIDTH  240
#define VECTOR_SIZE 5
#define MELON_RADIUS 30 //Diameter of 60
#define LEMON_RADIUS 25 //Diameter of 50
#define GRAPE_RADIUS 20 //Diameter of 40
#define APPLE_RADIUS 25 //Diameter of 50
#define TempPicturePtr(name,width,height) Picture name[(width)*(height)/6+2] = { {width,height,2} }

extern Point vector[VECTOR_SIZE];
extern const Picture background; // A 240x320 background image
extern const Picture blade; // A 3x3 background image
extern const Picture melon; // A 60x60 image of a melon
extern const Picture lemon; // A 50x50 image of a lemon
extern const Picture grape; // A 40x40 image of a grape
extern const Picture apple; // A 50x50 image of an apple
extern const Picture melon_cut; // A 60x60 image of a melon, cut in half
extern const Picture lemon_cut; // A 50x50 image of a lemon, cut in half
extern const Picture grape_cut; // A 40x40 image of a grape, cut in half
extern const Picture apple_cut; // A 50x50 image of an apple, cut in half
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
    is_cut = isCut(fr, vector[0]);

    if(is_cut == 1)
        fr.image = 'c';

    if(fr.image == 'n') {
        //erase60(fr.x, fr.y);
        update60(fr.x, fr.y, &melon);
    }
    else {
        //erase60(fr.x, fr.y);
        update60(fr.x, fr.y, &melon_cut);
        count++;
    }
    return is_cut;
}

int test_cut_lemon(int is_cut) {
    Point center = {.x = (SCREEN_WIDTH-120+LEMON_RADIUS), .y = 180+LEMON_RADIUS};
    Fruit fr = {.x = center.x, .y = center.y, .name = 'm', .image = 'n'};
    is_cut = isCut(fr, vector[0]);

    if(is_cut == 1)
        fr.image = 'c';

    if(fr.image == 'n') {
        //erase50(fr.x, fr.y);
        update50(fr.x, fr.y, &lemon);
    }
    else {
        //erase50(fr.x, fr.y);
        update50(fr.x, fr.y, &lemon_cut);
        count++;
    }
    return is_cut;
}

int test_cut_grape(int is_cut) {
    Point center = {.x = (SCREEN_WIDTH-120+GRAPE_RADIUS), .y = 120+GRAPE_RADIUS};
    Fruit fr = {.x = center.x, .y = center.y, .name = 'm', .image = 'n'};
    is_cut = isCut(fr, vector[0]);

    if(is_cut == 1)
        fr.image = 'c';

    if(fr.image == 'n') {
        //erase40(fr.x, fr.y);
        update40(fr.x, fr.y, &grape);
    }
    else {
        //erase40(fr.x, fr.y);
        update40(fr.x, fr.y, &grape_cut);
        count++;
    }
    return is_cut;
}

int test_cut_apple(int is_cut) {
    Point center = {.x = (SCREEN_WIDTH-120+APPLE_RADIUS), .y = 60+APPLE_RADIUS};
    Fruit fr = {.x = center.x, .y = center.y, .name = 'm', .image = 'n'};
    is_cut = isCut(fr, vector[0]);

    if(is_cut == 1)
        fr.image = 'c';

    if(fr.image == 'n') {
        update50(fr.x, fr.y, &apple);
    }
    else {
        update50(fr.x, fr.y, &apple_cut);
        count++;
    }
    return is_cut;
}

//ISR for reading x and y analog coords, storing the Point struct containing
//x and y into a global vector, and using update2() and erase() to animate
//the "blade" and its trajectory
//Note: The tail is best visible/clean when using a stylus instead of finger
void TIM15_IRQHandler  () {
    TIM15 -> SR &= ~TIM_SR_UIF;
    //Read x coordinate and output resulting pixel to SPI OLED
    int x_pixel = read_x();

    //Read y coordinate and output resulting pixel to SPI OLED
    int y_pixel = read_y();

    Point temp = {.x = x_pixel, .y = y_pixel};
    //Shift new point into index 0 of the vector
    //Allow fruit_ninja() to update drawing of this
    shift_into_vector(temp);

    meloncut = test_cut_melon(meloncut);
    lemoncut = test_cut_lemon(lemoncut);
    applecut = test_cut_apple(applecut);
    grapecut = test_cut_grape(grapecut);
    show_score(count);
    show_lives(3);
    if(count == 20)
        show_lives(2);
    else if(count == 40)
        show_lives(1);
    else if(count == 60) {
        show_lives(0);
        show_gameover_screen(count);
    }

//DEBUGGING AND VERIFICATION----------------------------------------------------
    //Print this first vector entry to OLED

//#define TEST_FIRST
#ifdef TEST_FIRST
    char string[21];
    snprintf(string, 21, "X Pixel: %03d", vector[0].x);
    spi1_display1(string);
    snprintf(string, 21, "Y Pixel: %03d", vector[0].y);
    spi1_display2(string);
#endif

//#define TEST_X
#ifdef TEST_X
    //Print x values of entire vector to OLED
    char string[21];
    snprintf(string, 21, "X%03d %03d %03d %03d %03d", vector[0].x, vector[1].x,
             vector[2].x, vector[3].x, vector[4].x);
    spi1_display1(string);
    snprintf(string, 21, " %03d %03d %03d %03d %03d", vector[5].x, vector[6].x,
             vector[7].x, vector[8].x, vector[9].x);
    spi1_display2(string);
#endif

//#define TEST_Y
#ifdef TEST_Y
    //Print y values of entire vector to OLED

    char string[21];
    snprintf(string, 21, "Y%03d %03d %03d %03d %03d", vector[0].y, vector[1].y,
             vector[2].y, vector[3].y, vector[4].y);
    spi1_display1(string);
    snprintf(string, 21, " %03d %03d %03d %03d %03d", vector[5].y, vector[6].y,
             vector[7].y, vector[8].y, vector[9].y);
    spi1_display2(string);
#endif
    //--------------------------------------------------------------------------
}

//Initialize the TIM15 ISR for reading the x and y coordinates and animating swipes
void init_tim15() {
    RCC -> APB2ENR |= RCC_APB2ENR_TIM15EN;
    TIM15 -> PSC = 120-1;
    TIM15-> ARR = 400-1;
    TIM15 -> DIER |= TIM_DIER_UIE;
    TIM15 -> CR1 |= TIM_CR1_CEN;
    NVIC -> ISER[0] |= 1<<TIM15_IRQn;
}
