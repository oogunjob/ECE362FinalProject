#include "stm32f0xx.h"
#include <stdio.h>
#include "swipe.h"
#include "oled.h"
#include "graphics.h"
#include "fruit.h"
#define SCREEN_HEIGHT 320
#define SCREEN_WIDTH  240
#define VECTOR_SIZE 15
#define MELON_RADIUS 30 //Diameter of 60
#define LEMON_RADIUS 25 //Diameter of 50
#define GRAPE_RADIUS 20 //Diameter of 40
#define APPLE_RADIUS 25 //Diameter of 50
#define TempPicturePtr(name,width,height) Picture name[(width)*(height)/6+2] = { {width,height,2} }

extern Point vector[VECTOR_SIZE];
extern const Picture background; // A 240x320 background image
extern const Picture blade;
extern const Picture melon; // A 60x60 image of a melon
extern const Picture lemon; // A 50x50 image of a lemon
extern const Picture grape; // A 40x40 image of a grape
extern const Picture apple; // A 50x50 image of an apple
extern const Picture melon_cut; // A 60x60 image of a melon, cut in half
extern const Picture lemon_cut; // A 50x50 image of a lemon, cut in half
extern const Picture grape_cut; // A 40x40 image of a grape, cut in half
extern const Picture apple_cut; // A 50x50 image of an apple, cut in half
int count = 0; //Simple counter used to determine how many times vector updated
int meloncut = 0;
int lemoncut = 0;
int grapecut = 0;
int applecut = 0;
int test_cut_melon(int is_cut);
int test_cut_lemon(int is_cut);
int test_cut_grape(int is_cut);
int test_cut_apple(int is_cut);


//NOTES:
//Screen Dims: 240 x 320

//VISUALIZATION

//         TOP OF SCREEN
//(0,320)-----------------(0,0)
//-----------------------------
//-----------------------------
//-----------------------------
//-----------------------------
//(240,320)-------------(240,0)

//TIM6 for DAC/DMA (NO ISR for DAC, needs to run at highest priority)
//TIM3 for read_x() and read_y() (which use ADC channels 8 and 9)
//OLED display functions in oled.h/oled.c
//Swipe mechanics and storage functions in swipe.h/swipe.c
//Graphics for output logic to LCD in graphics.c/graphics.h
//Actual graphical representations of objects in named .c files (e.g. background.c)
//LCD interfacing in lcd.c/lcd.h (DO NOT ALTER ANYTHING IN THESE)

//TO-DO:
//--Setup DAC
//--Setup game physics and fruit/bomb target graphics (background and blade
//  already set up)
//--Update only center of the fruit during motion, and count as "cut"
//  if swipe is within a specified tolerance (radius) of the center. This way,
//  no need to keep track of fruit borders
//--Attempt to use integer math for fruit trajectory (if not possible,
//  then use fixed-point)

//-----------------------------------------------
//               FUNCTION DECLARATIONS
//-----------------------------------------------
void init_tim3();
void nano_wait(unsigned int n);

//-----------------------------------------------
//                 INITIALIZATIONS
//-----------------------------------------------

//Initialize the TIM3 ISR for reading the x and y coordinates and animating swipes
void init_tim3() {
    RCC -> APB1ENR |= RCC_APB1ENR_TIM3EN;
    TIM3 -> PSC = 120-1;
    TIM3 -> ARR = 400-1;
    TIM3 -> DIER |= TIM_DIER_UIE;
    TIM3 -> CR1 |= TIM_CR1_CEN;
    NVIC -> ISER[0] |= 1<<TIM3_IRQn;
}

//-----------------------------------------------
//                TIMERS/ISR
//-----------------------------------------------

//ISR for reading x and y analog coords, storing the Point struct containing
//x and y into a global vector, and using update2() and erase() to animate
//the "blade" and its trajectory
//Note: The tail is best visible/clean when using a stylus instead of finger
void TIM3_IRQHandler() {
    TIM3 -> SR &= ~TIM_SR_UIF;
    //Read x coordinate and output resulting pixel to SPI OLED
    int x_pixel = read_x();

    //Read y coordinate and output resulting pixel to SPI OLED
    int y_pixel = read_y();


    //If screen detects a valid touch
    if(!(x_pixel == 0 || y_pixel == 0)) {
        Point temp = {.x = x_pixel, .y = y_pixel};
        if(count >= VECTOR_SIZE) {
            for(int i = VECTOR_SIZE - 1; i >= 0; i--)
                erase(vector[i].x, vector[i].y);
            count = 0;
        }
        //Shift new point into index 0 of the vector, increment point count
        shift_into_vector(temp);
        count++;
        update2(x_pixel, y_pixel);
        meloncut = test_cut_melon(meloncut);
        lemoncut = test_cut_lemon(lemoncut);
        grapecut = test_cut_grape(grapecut);
        applecut = test_cut_apple(applecut);
    }
    //If nothing is touching the screen, or stylus went out-of-bounds
    else {
        //Only erase everything in the vector. No need to update vector with
        //a 0-valued coordinate, increment count, or update blade position.
        for(int i = VECTOR_SIZE - 1; i >= 0; i--)
            erase(vector[i].x, vector[i].y);
    }

//DEBUGGING AND VERIFICATION----------------------------------------------------
    //Print this first vector entry to OLED

#define TEST_FIRST
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

//-----------------------------------------------
//                MISCELLANEOUS
//-----------------------------------------------
void test_draw_melon() {
    //Draw melon
    TempPicturePtr(tmp,MELON_RADIUS*2,MELON_RADIUS*2);
    pic_subset(tmp, &background, SCREEN_WIDTH-120, 240); // Copy the background
    pic_overlay(tmp,0,0,&melon,000); //Overlay transparent pixels that are color \000
    LCD_DrawPicture(SCREEN_WIDTH-120,240,tmp); //Draw new temporary picture

    //Draw cut melon
    TempPicturePtr(tmp2,MELON_RADIUS*2,MELON_RADIUS*2);
    pic_subset(tmp2, &background, SCREEN_WIDTH-180, 240); // Copy the background
    pic_overlay(tmp2,0,0,&melon_cut,000); //Overlay transparent pixels that are color \000
    LCD_DrawPicture(SCREEN_WIDTH-180,240,tmp2); //Draw new temporary picture
}

void test_draw_lemon() {
    //Draw lemon
    TempPicturePtr(tmp,LEMON_RADIUS*2,LEMON_RADIUS*2);
    pic_subset(tmp, &background, SCREEN_WIDTH-120, 180); // Copy the background
    pic_overlay(tmp,0,0,&lemon,000); //Overlay transparent pixels that are color \000
    LCD_DrawPicture(SCREEN_WIDTH-120,180,tmp); //Draw new temporary picture

    //Draw cut lemon
    TempPicturePtr(tmp2,LEMON_RADIUS*2,LEMON_RADIUS*2);
    pic_subset(tmp2, &background, SCREEN_WIDTH-180, 180); // Copy the background
    pic_overlay(tmp2,0,0,&lemon_cut,000); //Overlay transparent pixels that are color \000
    LCD_DrawPicture(SCREEN_WIDTH-180,180,tmp2); //Draw new temporary picture
}

void test_draw_grape() {
    //Draw grape
    TempPicturePtr(tmp,GRAPE_RADIUS*2,GRAPE_RADIUS*2);
    pic_subset(tmp, &background, SCREEN_WIDTH-120, 120); // Copy the background
    pic_overlay(tmp,0,0,&grape,000); //Overlay transparent pixels that are color \000
    LCD_DrawPicture(SCREEN_WIDTH-120,120,tmp); //Draw new temporary picture

    //Draw cut grape
    TempPicturePtr(tmp2,GRAPE_RADIUS*2,GRAPE_RADIUS*2);
    pic_subset(tmp2, &background, SCREEN_WIDTH-180, 120); // Copy the background
    pic_overlay(tmp2,0,0,&grape_cut,000); //Overlay transparent pixels that are color \000
    LCD_DrawPicture(SCREEN_WIDTH-180,120,tmp2); //Draw new temporary picture
}

void test_draw_apple() {
    //Draw apple
    TempPicturePtr(tmp,APPLE_RADIUS*2,APPLE_RADIUS*2);
    pic_subset(tmp, &background, SCREEN_WIDTH-120, 60); // Copy the background
    pic_overlay(tmp,0,0,&apple,000); //Overlay transparent pixels that are color \000
    LCD_DrawPicture(SCREEN_WIDTH-120,60,tmp); //Draw new temporary picture

    //Draw cut apple
    TempPicturePtr(tmp2,APPLE_RADIUS*2,APPLE_RADIUS*2);
    pic_subset(tmp2, &background, SCREEN_WIDTH-180, 60); // Copy the background
    pic_overlay(tmp2,0,0,&apple_cut,000); //Overlay transparent pixels that are color \000
    LCD_DrawPicture(SCREEN_WIDTH-180,60,tmp2); //Draw new temporary picture
}

int test_cut_melon(int is_cut) {
    Point center = {.x = (SCREEN_WIDTH-120+MELON_RADIUS), .y = 240+MELON_RADIUS};
    Fruit fr = {.center = center, .type = 'm'};
    is_cut = isCut(fr, vector[0]);
    if(is_cut == 0) {
        //Draw melon
        TempPicturePtr(tmp,MELON_RADIUS*2,MELON_RADIUS*2);
        pic_subset(tmp, &background, SCREEN_WIDTH-120, 240); // Copy the background
        pic_overlay(tmp,0,0,&melon,000); //Overlay transparent pixels that are color \000
        LCD_DrawPicture(SCREEN_WIDTH-120,240,tmp); //Draw new temporary picture
    }
    else {
        //Draw cut melon
        TempPicturePtr(tmp2,MELON_RADIUS*2,MELON_RADIUS*2);
        pic_subset(tmp2, &background, SCREEN_WIDTH-120, 240); // Copy the background
        pic_overlay(tmp2,0,0,&melon_cut,000); //Overlay transparent pixels that are color \000
        LCD_DrawPicture(SCREEN_WIDTH-120,240,tmp2); //Draw new temporary picture
    }
    return is_cut;
}

int test_cut_lemon(int is_cut) {
    Point center = {.x = (SCREEN_WIDTH-120+LEMON_RADIUS), .y = 180+LEMON_RADIUS};
    Fruit fr = {.center = center, .type = 'l'};
    is_cut = isCut(fr, vector[0]);
    if(is_cut == 0) {
        //Draw lemon
        TempPicturePtr(tmp,LEMON_RADIUS*2,LEMON_RADIUS*2);
        pic_subset(tmp, &background, SCREEN_WIDTH-120, 180); // Copy the background
        pic_overlay(tmp,0,0,&lemon,000); //Overlay transparent pixels that are color \000
        LCD_DrawPicture(SCREEN_WIDTH-120,180,tmp); //Draw new temporary picture
    }
    else {
        //Draw cut lemon
        TempPicturePtr(tmp2,LEMON_RADIUS*2,LEMON_RADIUS*2);
        pic_subset(tmp2, &background, SCREEN_WIDTH-120, 180); // Copy the background
        pic_overlay(tmp2,0,0,&lemon_cut,000); //Overlay transparent pixels that are color \000
        LCD_DrawPicture(SCREEN_WIDTH-120,180,tmp2); //Draw new temporary picture
    }
    return is_cut;
}

int test_cut_grape(int is_cut) {
    Point center = {.x = (SCREEN_WIDTH-120+GRAPE_RADIUS), .y = 120+GRAPE_RADIUS};
    Fruit fr = {.center = center, .type = 'g'};
    is_cut = isCut(fr, vector[0]);
    if(is_cut == 0) {
        //Draw grape
        TempPicturePtr(tmp,GRAPE_RADIUS*2,GRAPE_RADIUS*2);
        pic_subset(tmp, &background, SCREEN_WIDTH-120, 120); // Copy the background
        pic_overlay(tmp,0,0,&grape,000); //Overlay transparent pixels that are color \000
        LCD_DrawPicture(SCREEN_WIDTH-120,120,tmp); //Draw new temporary picture
    }
    else {
        //Draw cut grape
        TempPicturePtr(tmp2,GRAPE_RADIUS*2,GRAPE_RADIUS*2);
        pic_subset(tmp2, &background, SCREEN_WIDTH-120, 120); // Copy the background
        pic_overlay(tmp2,0,0,&grape_cut,000); //Overlay transparent pixels that are color \000
        LCD_DrawPicture(SCREEN_WIDTH-120,120,tmp2); //Draw new temporary picture
    }
    return is_cut;
}

int test_cut_apple(int is_cut) {
    Point center = {.x = (SCREEN_WIDTH-120+APPLE_RADIUS), .y = 60+APPLE_RADIUS};
    Fruit fr = {.center = center, .type = 'a'};
    is_cut = isCut(fr, vector[0]);
    if(is_cut == 0) {
        //Draw apple
        TempPicturePtr(tmp,APPLE_RADIUS*2,APPLE_RADIUS*2);
        pic_subset(tmp, &background, SCREEN_WIDTH-120, 60); // Copy the background
        pic_overlay(tmp,0,0,&apple,000); //Overlay transparent pixels that are color \000
        LCD_DrawPicture(SCREEN_WIDTH-120,60,tmp); //Draw new temporary picture
    }
    else {
        //Draw cut apple
        TempPicturePtr(tmp2,APPLE_RADIUS*2,APPLE_RADIUS*2);
        pic_subset(tmp2, &background, SCREEN_WIDTH-120, 60); // Copy the background
        pic_overlay(tmp2,0,0,&apple_cut,000); //Overlay transparent pixels that are color \000
        LCD_DrawPicture(SCREEN_WIDTH-120,60,tmp2); //Draw new temporary picture
    }
    return is_cut;
}

int main() {
    LCD_Setup(); // this will call init_lcd_spi()
    //Draw background with upper left corner at (0,0)
    LCD_DrawPicture(0,0,&background);
    //test_draw_melon();
    //test_draw_lemon();
    //test_draw_grape();
    //test_draw_apple();


    init_adc();
    init_spi1();
    spi1_init_oled();
    init_reads();
    //init_tim3() is ALWAYS the last call in main
    init_tim3();
    for(;;)
        ;
}
