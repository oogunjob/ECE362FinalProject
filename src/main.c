#include "stm32f0xx.h"
#include <stdio.h>
#include "swipe.h"
#include "oled.h"
#define SCREEN_HEIGHT 320
#define SCREEN_WIDTH  240
#define VECTOR_SIZE 10

extern Point vector[VECTOR_SIZE];

//NOTES:
//Screen Dims: 240 x 320
//TIM6 for DAC/DMA (NO ISR for DAC)
//TIM3 for read_x() and read_y() (which use ADC channels 8 and 9)
//OLED display functions in oled.h/oled.c
//Swipe mechanics and storage functions in swipe.h/swipe.c

//TO-DO:
//--Setup DAC
//--Setup game physics and graphics
//--Update only center of the fruit during motion, count as "cut"
//  if swipe is within a specified tolerance of the center. This way,
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

//Initialize the TIM3 ISR for reading the x and y coordinates
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

//ISR for reading x and y analog coords
void TIM3_IRQHandler() {
    TIM3 -> SR &= ~TIM_SR_UIF;
    //Read x coordinate and output resulting pixel to SPI OLED
    int x_pixel = read_x();
    nano_wait(500000);  //500 us between reads

    //Read y coordinate and output resulting pixel to SPI OLED
    int y_pixel = read_y();
    nano_wait(500000);  //500 us between reads

    Point temp = {.x = x_pixel, .y = y_pixel};
    shift_into_vector(temp);

    //Expect to see this created temp point as first vector entry

//DEBUGGING AND VERIFICATION----------------------------------------------------
    //Print this first vector entry

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
    //Print x values of entire vector
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
    //Print y values of entire vector

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
    init_adc();
    init_spi1();
    spi1_init_oled();
    init_reads();
    init_tim3();
    for(;;)
        ;
}
