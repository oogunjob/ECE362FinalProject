#include "stm32f0xx.h"
#include <stdio.h>
#include "swipe.h"
#include "oled.h"
#include "graphics.h"
#define SCREEN_HEIGHT 320
#define SCREEN_WIDTH  240
#define VECTOR_SIZE 15

extern Point vector[VECTOR_SIZE];
extern const Picture background; // A 240x320 background image
int count = 0; //Simple counter used to determine how many times vector updated

//NOTES:
//Screen Dims: 240 x 320
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

//RECOMMENDATION: Put some logic in here flagging a (0,0) touch as invalid for
//a swipe, and otherwise flag a (x,y) touch within the bounds (240,320) as valid
//(Otherwise, any fruit entering near (0,0) would be sliced even if not touching
//the screen)
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
                erase(SCREEN_WIDTH - vector[i].x, vector[i].y);
            count = 0;
        }
        //Shift new point into index 0 of the vector, increment point count
        shift_into_vector(temp);
        count++;
        update2(SCREEN_WIDTH - x_pixel, y_pixel);
    }
    //If nothing is touching the screen, or stylus went out-of-bounds
    else {
        //Only erase everything in the vector. No need to update vector with
        //a 0-valued coordinate, increment count, or update blade position.
        for(int i = VECTOR_SIZE - 1; i >= 0; i--)
            erase(SCREEN_WIDTH - vector[i].x, vector[i].y);
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


int main() {
    LCD_Setup(); // this will call init_lcd_spi()
    //Draw background with upper left corner at (0,0)
    LCD_DrawPicture(0,0,&background);
    init_adc();
    init_spi1();
    spi1_init_oled();
    init_reads();
    //init_tim3() is ALWAYS the last call in main
    init_tim3();
    for(;;)
        ;
}
