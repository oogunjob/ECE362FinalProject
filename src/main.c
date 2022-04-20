#include "stm32f0xx.h"
#include <stdio.h>
#include "oled.h"
#include "fruit_ninja.h"
#include "isr.h"
#define SCREEN_HEIGHT 320
#define SCREEN_WIDTH  240
#define VECTOR_SIZE 15

extern const Picture background; // A 240x320 background image
extern const Picture blade; // A 3x3 pixelated image to indicate where swipe is

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
//TIM15 for read_x() and read_y() (which use ADC channels 8 and 9) and seeding random numbers
//OLED display functions in oled.h/oled.c
//Swipe mechanics and storage functions in swipe.h/swipe.c
//Graphics for output logic to LCD in graphics.c/graphics.h
//Actual graphical representations of objects in named .c files (e.g. background.c)
//LCD interfacing in lcd.c/lcd.h (DO NOT ALTER ANYTHING IN THESE)

//TODO:
//--Setup DAC
//--Update only center of the fruit during motion, and count as "cut"
//  if swipe is within a specified tolerance (radius) of the center. This way,
//  no need to keep track of fruit borders
//--Attempt to use integer math for fruit trajectory (if not possible,
//  then use fixed-point)

int main() {
    LCD_Setup(); // this will call init_lcd_spi()
    //Draw background with upper right corner at (0,0)
    LCD_DrawPicture(0,0,&background);
    init_adc();
    init_spi1();
    spi1_init_oled();
    init_reads();
    //fruit_ninja();
    init_tim15();
    for(;;)
        ;
}
