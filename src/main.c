#include "stm32f0xx.h"
#include <stdio.h>
#include "oled.h"
#include "fruit_ninja.h"
#include "isr.h"
#define SCREEN_HEIGHT 320
#define SCREEN_WIDTH  240
#define VECTOR_SIZE 15

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

//-----------------------------------------------
//               FUNCTION DECLARATIONS
//-----------------------------------------------
void nano_wait(unsigned int n);

//-----------------------------------------------
//                MISCELLANEOUS
//-----------------------------------------------


int main() {
    fruit_ninja();
    //init_tim15();
    for(;;)
        ;
}
